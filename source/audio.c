#include "audio.h"
#include "juego.h"
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Las pistas BGM mono a 22050 Hz caben precargadas y evitan pausas al cambiar de estado.
#define MAX_BGM_BYTES (12 * 1024 * 1024)

typedef struct {
    u8* data;
    u32 numSamples;
    u32 sampleRate;
    u16 channels;
    u16 bitsPerSample;
    u16 ndspFormat;
    bool cargado;
} WavClip;

static bool audioDisponible = false;
static WavClip clipsSFX[MAX_SFX];
static WavClip clipsBGM[2];

static ndspWaveBuf waveBufSFX;
static ndspWaveBuf waveBufBGM;
static int bgmActual = -1;

static const char* RUTAS_SFX[MAX_SFX] = {
    "romfs:/audio/golpe_jugador.wav",
    "romfs:/audio/golpe_enemigo.wav",
    "romfs:/audio/habilidad_espada.wav",
    "romfs:/audio/defender.wav",
    "romfs:/audio/pocion.wav"
};

static const char* RUTAS_BGM[2] = {
    "romfs:/audio/exploracion.wav",
    "romfs:/audio/combate.wav"
};

static void liberarClip(WavClip* clip) {
    if (clip->data) {
        linearFree(clip->data);
        clip->data = NULL;
    }
    clip->cargado = false;
}

static bool cargarArchivoWav(const char* ruta, WavClip* clip, u32 maxBytes) {
    memset(clip, 0, sizeof(WavClip));
    FILE* f = fopen(ruta, "rb");
    if (!f) return false;

    char id[4];
    u32 fileSize = 0;
    char waveFmt[4];

    if (fread(id, 1, 4, f) != 4 || memcmp(id, "RIFF", 4) != 0) { fclose(f); return false; }
    fread(&fileSize, 4, 1, f);
    if (fread(waveFmt, 1, 4, f) != 4 || memcmp(waveFmt, "WAVE", 4) != 0) { fclose(f); return false; }

    u16 audioFormat = 1, numChannels = 1, bitsPerSample = 16;
    u32 sampleRate = 22050;
    u8* pcmData = NULL;
    u32 dataSize = 0;

    while (!feof(f)) {
        char chunkId[4];
        u32 chunkSize = 0;
        if (fread(chunkId, 1, 4, f) != 4) break;
        if (fread(&chunkSize, 4, 1, f) != 1) break;

        if (memcmp(chunkId, "fmt ", 4) == 0) {
            u32 byteRate;
            u16 blockAlign;
            fread(&audioFormat, 2, 1, f);
            fread(&numChannels, 2, 1, f);
            fread(&sampleRate, 4, 1, f);
            fread(&byteRate, 4, 1, f);
            fread(&blockAlign, 2, 1, f);
            fread(&bitsPerSample, 2, 1, f);
            if (chunkSize > 16) {
                u32 extra = chunkSize - 16;
                fseek(f, extra + (extra & 1), SEEK_CUR);
            }
        } else if (memcmp(chunkId, "data", 4) == 0) {
            u32 bytesALeer = chunkSize;
            if (maxBytes > 0 && bytesALeer > maxBytes) {
                bytesALeer = maxBytes;
            }
            // Si queda poca RAM lineal, ajustarse al bloque libre disponible
            u32 libre = linearSpaceFree();
            if (libre > 65536 && bytesALeer > libre - 65536) {
                bytesALeer = (libre - 65536) & ~3u;
            }

            // Si el WAV viene en 32-bit float (audioFormat == 3) o 24-bit PCM, convertir al vuelo a PCM16
            if ((audioFormat == 3 && bitsPerSample == 32) || (audioFormat == 1 && bitsPerSample == 24)) {
                u32 bytesPorMuestraOrig = bitsPerSample / 8;
                u32 totalMuestras = bytesALeer / bytesPorMuestraOrig;
                dataSize = totalMuestras * sizeof(s16);
                pcmData = (u8*)linearAlloc(dataSize);
                if (pcmData) {
                    s16* dst16 = (s16*)pcmData;
                    for (u32 i = 0; i < totalMuestras; i++) {
                        if (audioFormat == 3) {
                            float val = 0.0f;
                            fread(&val, 4, 1, f);
                            if (val > 1.0f) val = 1.0f;
                            if (val < -1.0f) val = -1.0f;
                            dst16[i] = (s16)(val * 32767.0f);
                        } else {
                            u8 b[3];
                            fread(b, 1, 3, f);
                            dst16[i] = (s16)((b[2] << 8) | b[1]);
                        }
                    }
                    audioFormat = 1;
                    bitsPerSample = 16;
                }
            } else {
                bytesALeer &= ~3u; // Alinear a 4 bytes
                pcmData = (u8*)linearAlloc(bytesALeer);
                if (pcmData) {
                    dataSize = fread(pcmData, 1, bytesALeer, f);
                }
            }
            break;
        } else {
            // Respetar el byte de relleno par del estándar RIFF
            fseek(f, chunkSize + (chunkSize & 1), SEEK_CUR);
        }
    }
    fclose(f);

    if (!pcmData || dataSize == 0) {
        if (pcmData) linearFree(pcmData);
        return false;
    }

    u32 bytesPerSample = (bitsPerSample / 8) * numChannels;
    if (bytesPerSample == 0) bytesPerSample = 2;

    clip->data          = pcmData;
    clip->numSamples    = dataSize / bytesPerSample;
    clip->sampleRate    = sampleRate;
    clip->channels      = numChannels;
    clip->bitsPerSample = bitsPerSample;

    if (bitsPerSample == 8) {
        clip->ndspFormat = (numChannels == 2) ? NDSP_FORMAT_STEREO_PCM8 : NDSP_FORMAT_MONO_PCM8;
    } else {
        clip->ndspFormat = (numChannels == 2) ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_MONO_PCM16;
    }

    DSP_FlushDataCache(clip->data, dataSize);
    clip->cargado = true;
    return true;
}

void audioInit(void) {
    if (R_FAILED(ndspInit())) {
        audioDisponible = false;
        return;
    }

    audioDisponible = true;
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);

    float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = 1.0f;
    mix[1] = 1.0f;

    for (int ch = 0; ch <= 1; ch++) {
        ndspChnReset(ch);
        ndspChnSetInterp(ch, NDSP_INTERP_LINEAR);
        ndspChnSetMix(ch, mix);
    }

    for (int i = 0; i < MAX_SFX; i++) {
        cargarArchivoWav(RUTAS_SFX[i], &clipsSFX[i], 512 * 1024);
    }

    for (int i = 0; i < 2; i++) {
        cargarArchivoWav(RUTAS_BGM[i], &clipsBGM[i], MAX_BGM_BYTES);
    }

    bgmActual = -1;
    audioCambiarBGM(BGM_MAPA);
}

void audioCambiarBGM(TipoBGM bgm) {
    if (!audioDisponible) return;

    int idx = (bgm == BGM_COMBATE) ? 1 : 0;
    if (bgmActual == idx && clipsBGM[idx].cargado && ndspChnIsPlaying(1)) return;

    ndspChnWaveBufClear(1);

    if (!clipsBGM[idx].cargado) {
        bgmActual = -1;
        return;
    }

    bgmActual = idx;
    ndspChnSetRate(1, (float)clipsBGM[idx].sampleRate);
    ndspChnSetFormat(1, clipsBGM[idx].ndspFormat);

    float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = 0.85f;
    mix[1] = 0.85f;
    ndspChnSetMix(1, mix);

    memset(&waveBufBGM, 0, sizeof(ndspWaveBuf));
    waveBufBGM.data_vaddr = clipsBGM[idx].data;
    waveBufBGM.nsamples   = clipsBGM[idx].numSamples;
    waveBufBGM.looping    = true;

    ndspChnWaveBufAdd(1, &waveBufBGM);
}

void audioActualizarEstado(int estadoJuego) {
    if (!audioDisponible) return;
    if (estadoJuego == ESTADO_COMBATE) {
        audioCambiarBGM(BGM_COMBATE);
    } else {
        audioCambiarBGM(BGM_MAPA);
    }
}

void audioReproducirSFX(TipoSFX tipo) {
    if (!audioDisponible || tipo < 0 || tipo >= MAX_SFX) return;
    WavClip* clip = &clipsSFX[tipo];
    if (!clip->cargado) return;

    ndspChnWaveBufClear(0);
    ndspChnSetRate(0, (float)clip->sampleRate);
    ndspChnSetFormat(0, clip->ndspFormat);

    float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = 1.0f;
    mix[1] = 1.0f;
    ndspChnSetMix(0, mix);

    memset(&waveBufSFX, 0, sizeof(ndspWaveBuf));
    waveBufSFX.data_vaddr = clip->data;
    waveBufSFX.nsamples   = clip->numSamples;
    waveBufSFX.looping    = false;

    ndspChnWaveBufAdd(0, &waveBufSFX);
}

void audioExit(void) {
    if (!audioDisponible) return;

    ndspChnWaveBufClear(0);
    ndspChnWaveBufClear(1);

    for (int i = 0; i < MAX_SFX; i++) {
        liberarClip(&clipsSFX[i]);
    }
    for (int i = 0; i < 2; i++) {
        liberarClip(&clipsBGM[i]);
    }

    ndspExit();
    audioDisponible = false;
}