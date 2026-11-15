#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

typedef enum {
    SFX_GOLPE_JUGADOR = 0,
    SFX_GOLPE_ENEMIGO,
    SFX_HABILIDAD_ESPADA,
    SFX_DEFENDER,
    SFX_POCION,
    MAX_SFX
} TipoSFX;

typedef enum {
    BGM_MAPA = 0,
    BGM_COMBATE,
    BGM_VICTORIA,
    MAX_BGM
} TipoBGM;

void audioInit(void);
void audioExit(void);
void audioReproducirSFX(TipoSFX tipo);
void audioCambiarBGM(TipoBGM bgm);
void audioActualizarEstado(int estadoJuego);

#endif