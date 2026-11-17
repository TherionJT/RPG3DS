#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// 1 = reactiva el creador de personaje por capas (oculto por ahora; X en la seleccion lo abre)
#define USAR_PERSONALIZACION 0

#include "heroes_t3x.h"    // 8 viajeros: 4 direcciones x 4 cuadros (gfx/heroes.t3s)
#include "combate_ophilia_t3x.h"
#include "combate_cyrus_t3x.h"
#include "combate_tressa_t3x.h"
#include "combate_olberic_t3x.h"
#include "combate_primrose_t3x.h"
#include "combate_alfyn_t3x.h"
#include "combate_therion_t3x.h"
#include "combate_haanit_t3x.h"
#include "fx_t3x.h"         // efectos de combate (gfx/fx.t3s)
#include "combate_meta.h"   // anclas y alcance de los cuadros de combate
#if USAR_PERSONALIZACION
#include "personaje_t3x.h" // Hoja del personaje por capas (gfx/personaje.t3s)
#endif

typedef enum {
    ESTADO_SELECCION,
    ESTADO_PERSONALIZACION,
    ESTADO_EXPLORACION,
    ESTADO_COMBATE
} EstadoJuego;

typedef struct {
    char nombre[20];
    int hp;
    int maxHp;
    int atk;
    int def;
    bool defendiendo;
} Entidad;

// Función auxiliar para dibujar texto dinámico con la fuente del sistema de 3DS
// El simbolo '¡' se dibuja a mano porque la fuente compartida de Azahar no lo incluye.
void dibujarTexto(C2D_TextBuf buf, float x, float y, float escala, u32 color, const char* str) {
    const char* p = str;
    char seg[128];
    while (*p) {
        const char* q = strstr(p, "¡");
        size_t n = q ? (size_t)(q - p) : strlen(p);
        if (n > 0) {
            size_t c = n < sizeof(seg) - 1 ? n : sizeof(seg) - 1;
            memcpy(seg, p, c);
            seg[c] = '\0';
            C2D_Text text;
            C2D_TextParse(&text, buf, seg);
            C2D_TextOptimize(&text);
            C2D_DrawText(&text, C2D_WithColor, x, y, 0.0f, escala, escala, color);
            float ancho = 0.0f;
            C2D_TextGetDimensions(&text, escala, escala, &ancho, NULL);
            x += ancho;
        }
        if (!q) break;
        // '¡' = punto arriba + barra debajo
        float h = 30.0f * escala;
        float g = roundf(h * 0.11f); if (g < 1.0f) g = 1.0f;
        float gx = roundf(x + h * 0.08f);
        C2D_DrawRectSolid(gx, roundf(y + h * 0.30f), 0.0f, g, g, color);
        C2D_DrawRectSolid(gx, roundf(y + h * 0.30f) + 2 * g, 0.0f, g, roundf(h * 0.42f), color);
        x += g + h * 0.2f;
        p = q + strlen("¡");
    }
}

// ==========================================
// PERSONAJE POR CAPAS (sprite base 32x32)
// ==========================================
// Orden de las imagenes dentro de personaje.t3s: 8 direcciones x 3 cuadros x 9 capas.
// Las capas en gris se colorean en el juego con un tinte multiplicativo.
#define PJ_TAM            32
#define PJ_CAPAS_X_CUADRO 9
#define PJ_CUADROS_X_DIR  9   // 0 = quieto, 1..8 = ciclo de caminata
#define NUM_DIRS          8
// Direcciones en el mismo orden que la hoja (sentido horario empezando mirando abajo)
enum { DIR_S = 0, DIR_SO, DIR_O, DIR_NO, DIR_N, DIR_NE, DIR_E, DIR_SE };
enum {
    CAPA_PIERNAS = 0, CAPA_CUERPO, CAPA_CARA,
    CAPA_ROPA_0,          // + estilo de ropa (0..2)
    CAPA_PELO_0 = 6       // + peinado (0..2)
};
enum { CUADRO_QUIETO = 0, CUADRO_CAMINATA_0 = 1 };
#define CUADROS_CAMINATA 8      // contacto, baja, paso, sube (x2 pies)

// Movimiento suave en el mapa
#define VEL_MAX       1.5f      // px por frame a velocidad crucero
#define ACELERACION   0.25f     // px/frame^2 al arrancar (~6 frames hasta crucero)
#define FRENADO       0.35f     // px/frame^2 al soltar
#define PX_POR_CUADRO 3.5f      // la animacion avanza segun la distancia recorrida (sin patinar)
#define FRAMES_X_GIRO 3         // frames por cada paso de 45 grados al girar

#if USAR_PERSONALIZACION
#define NUM_PIELES       5
#define NUM_PEINADOS     3
#define NUM_COLORES_PELO 8
#define NUM_ROPAS        3
#define NUM_COLORES_ROPA 8
#define NUM_NOMBRES      6

typedef struct { const char* nombre; u8 r, g, b; } ColorNombrado;

static const ColorNombrado PIELES[NUM_PIELES] = {
    {"Clara", 255, 219, 180}, {"Media", 240, 188, 148}, {"Trigueña", 200, 140, 100},
    {"Morena", 150, 100, 70}, {"Oscura", 105, 70, 50}
};
static const ColorNombrado COLORES_PELO[NUM_COLORES_PELO] = {
    {"Castaño", 120, 72, 40}, {"Negro", 48, 44, 56}, {"Rubio", 250, 210, 90},
    {"Pelirrojo", 205, 85, 40}, {"Blanco", 235, 235, 240}, {"Azul", 70, 110, 230},
    {"Rosa", 240, 120, 180}, {"Verde", 80, 180, 90}
};
static const ColorNombrado COLORES_ROPA[NUM_COLORES_ROPA] = {
    {"Azul", 60, 110, 220}, {"Rojo", 200, 50, 50}, {"Verde", 60, 160, 80},
    {"Morado", 130, 70, 200}, {"Plata", 205, 205, 215}, {"Negro", 70, 70, 80},
    {"Dorado", 230, 190, 60}, {"Marrón", 140, 90, 50}
};
static const char* PEINADOS[NUM_PEINADOS] = {"Corto", "Largo", "En puntas"};
static const char* ROPAS[NUM_ROPAS]       = {"Túnica", "Armadura", "Toga de mago"};
static const char* NOMBRES[NUM_NOMBRES]   = {"Heroe", "Aria", "Kael", "Luna", "Ren", "Sol"};

typedef struct {
    int piel, peinado, colorPelo, ropa, colorRopa;
} Apariencia;

static u32 colorDe(const ColorNombrado* c) { return C2D_Color32(c->r, c->g, c->b, 255); }


static void dibujarCapa(C2D_SpriteSheet hoja, int dir, int cuadro, int capa, float x, float y, float escala, const C2D_ImageTint* tinte) {
    C2D_Image img = C2D_SpriteSheetGetImage(hoja, (dir * PJ_CUADROS_X_DIR + cuadro) * PJ_CAPAS_X_CUADRO + capa);
    C2D_DrawImageAt(img, x, y, 0.5f, tinte, escala, escala);
}

// Dibuja el personaje completo (esquina superior izquierda en x,y). Usar escalas enteras.
static void dibujarPersonaje(C2D_SpriteSheet hoja, const Apariencia* ap, int dir, int cuadro, float x, float y, float escala) {
    C2D_ImageTint tPiel, tPelo, tRopa;
    C2D_PlainImageTint(&tPiel, colorDe(&PIELES[ap->piel]), 1.0f);
    C2D_PlainImageTint(&tPelo, colorDe(&COLORES_PELO[ap->colorPelo]), 1.0f);
    C2D_PlainImageTint(&tRopa, colorDe(&COLORES_ROPA[ap->colorRopa]), 1.0f);

    x = floorf(x + 0.5f); y = floorf(y + 0.5f); // posicion subpixel -> pixel entero (sin parpadeo)
    C2D_SetTintMode(C2D_TintMult);
    dibujarCapa(hoja, dir, cuadro, CAPA_PIERNAS, x, y, escala, NULL);
    dibujarCapa(hoja, dir, cuadro, CAPA_CUERPO, x, y, escala, &tPiel);
    dibujarCapa(hoja, dir, cuadro, CAPA_ROPA_0 + ap->ropa, x, y, escala, &tRopa);
    dibujarCapa(hoja, dir, cuadro, CAPA_CARA, x, y, escala, NULL);
    dibujarCapa(hoja, dir, cuadro, CAPA_PELO_0 + ap->peinado, x, y, escala, &tPelo);
    C2D_SetTintMode(C2D_TintSolid);
}

// Ciclo de caminata: quieto, paso izq, quieto, paso der
static int cuadroPorDistancia(float distancia) {
    return CUADRO_CAMINATA_0 + ((int)(distancia / PX_POR_CUADRO)) % CUADROS_CAMINATA;
}

#endif // USAR_PERSONALIZACION

// ==========================================
// VIAJEROS (sprites de 64 px de alto, gfx/heroes.t3s)
// ==========================================
// Orden en la hoja: heroe * 16 + direccion * 4 + cuadro
#define NUM_HEROES          8
#define HEROE_CUADROS       4
#define PX_POR_CUADRO_HEROE 11.25f  // a VEL_MAX (1.5 px/frame) da 8 cuadros por segundo
enum { HDIR_ABAJO = 0, HDIR_IZQ, HDIR_DER, HDIR_ARRIBA };

typedef struct {
    const char* nombre;
    const char* clase;
    const char* lema;
    int hp, atk, def;
    int ancho, alto;    // tamano del cuadro
    int anclaX, anclaY; // punto de los pies dentro del cuadro
    int anchoCuerpo;    // para la sombra
} Heroe;

static const Heroe HEROES[NUM_HEROES] = {
    {"Ophilia", "Clériga", "Lleva la llama sagrada", 95, 14, 6, 42, 64, 21, 63, 22},
    {"Cyrus", "Erudito", "Sabio de la gran academia", 90, 15, 5, 38, 64, 19, 63, 20},
    {"Tressa", "Mercader", "Comercia con buen ojo", 100, 16, 6, 39, 64, 19, 63, 19},
    {"Olberic", "Guerrero", "Espada firme y leal", 125, 20, 8, 50, 64, 26, 64, 28},
    {"Primrose", "Bailarina", "Baila entre las sombras", 90, 17, 4, 43, 64, 21, 63, 16},
    {"Alfyn", "Boticario", "Cura con hierbas y valor", 105, 16, 6, 43, 64, 23, 63, 22},
    {"Therion", "Ladrón", "Ninguna cerradura lo frena", 95, 19, 4, 39, 64, 19, 63, 18},
    {"H'aanit", "Cazadora", "Caza junto a su fiera", 100, 18, 5, 44, 64, 22, 64, 20},
};

// El juego se mueve en 8 direcciones; las diagonales usan el perfil mas cercano.
static int dirHeroe(int dir8) {
    switch (dir8) {
        case DIR_S: return HDIR_ABAJO;
        case DIR_N: return HDIR_ARRIBA;
        case DIR_SO: case DIR_O: case DIR_NO: return HDIR_IZQ;
        default: return HDIR_DER;
    }
}

// Dibuja al heroe con los pies en (x, y). Usar escalas enteras.
static void dibujarHeroe(C2D_SpriteSheet hoja, int h, int dir4, int cuadro, float x, float y, float escala) {
    const Heroe* he = &HEROES[h];
    C2D_Image img = C2D_SpriteSheetGetImage(hoja, h * 16 + dir4 * HEROE_CUADROS + cuadro);
    C2D_DrawImageAt(img, floorf(x - he->anclaX * escala + 0.5f), floorf(y - he->anclaY * escala + 0.5f), 0.5f, NULL, escala, escala);
}

static void dibujarSombra(float x, float y, float ancho, u32 color) {
    float w = ancho * 1.5f, h = w * 0.3f;
    if (h < 4.0f) h = 4.0f;
    C2D_DrawEllipseSolid(floorf(x - w / 2), floorf(y - h / 2), 0.0f, w, h, color);
}

// Ciclo de 4 cuadros segun la distancia (el primer paso se ve al instante); quieto = cuadro 0
static int cuadroHeroe(bool caminando, float distancia) {
    if (!caminando) return 0;
    return (1 + (int)(distancia / PX_POR_CUADRO_HEROE)) % HEROE_CUADROS;
}

static float anchoTexto(C2D_TextBuf buf, float escala, const char* str) {
    C2D_Text t; float w = 0.0f;
    C2D_TextParse(&t, buf, str);
    C2D_TextGetDimensions(&t, escala, escala, &w, NULL);
    return w;
}

static void dibujarTextoCentrado(C2D_TextBuf buf, float cx, float y, float escala, u32 color, const char* str) {
    dibujarTexto(buf, floorf(cx - anchoTexto(buf, escala, str) / 2), y, escala, color, str);
}

static void dibujarBarra(float x, float y, float w, float h, float frac, u32 fondo, u32 color) {
    C2D_DrawRectSolid(x, y, 0.0f, w, h, fondo);
    C2D_DrawRectSolid(x, y, 0.0f, w * frac, h, color);
}

// ==========================================
// ANIMACIONES DE COMBATE (gfx/combate_<viajero>.t3s + gfx/fx.t3s)
// ==========================================
// 18 cuadros por viajero, mirando a la derecha (ver combate_meta.h)
enum { CC_IDLE = 0, CC_ATAQUE = 4, CC_DEFENSA = 10, CC_HERIDO = 12, CC_VICTORIA = 14, CC_KO = 16 };
typedef enum { EST_TAJO, EST_ESTOCADA, EST_DOBLE, EST_GIRO, EST_HACHA, EST_FLECHA, EST_ORBE, EST_LUZ } EstiloAtaque;
// Ophilia, Cyrus, Tressa, Olberic, Primrose, Alfyn, Therion, H'aanit
static const EstiloAtaque ESTILO_HEROE[NUM_HEROES] = {
    EST_LUZ, EST_ORBE, EST_ESTOCADA, EST_TAJO, EST_GIRO, EST_HACHA, EST_DOBLE, EST_FLECHA
};
static bool esCuerpoACuerpo(EstiloAtaque e) { return e <= EST_HACHA; }

// Duracion en frames (60 fps) de los 6 cuadros del ataque: carga, impulso, golpe, impacto, seguimiento, recuperar
static const u8 DUR_ATAQUE[6] = {6, 9, 4, 6, 7, 9};
#define CUADRO_GOLPE    3   // el arma conecta / se suelta el proyectil al empezar este cuadro
#define FR_AVANCE      14   // carrera hacia el enemigo (y regreso)
#define FR_ENE_AVANCE  18
#define FR_ENE_GOLPE   18
#define FR_VICTORIA   140
#define FR_DERROTA    150
#define ESCALA_COMBATE 2.0f
#define JUG_X  92.0f        // pies del viajero en combate
#define JUG_Y 196.0f
#define ENE_X 275.0f        // esquina del enemigo
#define ENE_Y  95.0f
#define ENE_TAM 55.0f

static int inicioCuadroAtaque(int c) { int a = 0; for (int i = 0; i < c; i++) a += DUR_ATAQUE[i]; return a; }
static int cuadroAtaque(int t) { for (int i = 0; i < 6; i++) if (t < inicioCuadroAtaque(i + 1)) return i; return 5; }
static float suave(float k) { if (k < 0) k = 0; if (k > 1) k = 1; return k * k * (3.0f - 2.0f * k); }

// Profundidad 0: todo el combate se pinta en orden (el cuadro transparente no tapa al enemigo ni a los textos)
static void dibujarHeroeCombate(C2D_SpriteSheet hoja, int h, int cuadro, float x, float y, float escala, const C2D_ImageTint* tinte) {
    const MetaCombate* m = &META_COMBATE[h];
    C2D_Image img = C2D_SpriteSheetGetImage(hoja, cuadro);
    C2D_DrawImageAt(img, floorf(x - m->ax * escala + 0.5f), floorf(y - m->ay * escala + 0.5f), 0.0f, tinte, escala, escala);
}

// --- Efectos (tajos, impactos, luz...) ---
#define MAX_FX 12
typedef struct { bool activo; int base, num, dur, t; float x, y, vx, escala; } Efecto;
static Efecto efectos[MAX_FX];
static void lanzarFx(int base, int num, int dur, float x, float y, float vx) {
    for (int i = 0; i < MAX_FX; i++) if (!efectos[i].activo) {
        efectos[i] = (Efecto){true, base, num, dur, 0, x, y, vx, ESCALA_COMBATE};
        return;
    }
}
static void actualizarFx(void) {
    for (int i = 0; i < MAX_FX; i++) if (efectos[i].activo) {
        efectos[i].x += efectos[i].vx;
        if (++efectos[i].t >= efectos[i].num * efectos[i].dur) efectos[i].activo = false;
    }
}
static void dibujarImagenCentrada(C2D_SpriteSheet hoja, int idx, float x, float y, float esc) {
    C2D_Image img = C2D_SpriteSheetGetImage(hoja, idx);
    C2D_DrawImageAt(img, floorf(x - img.subtex->width * esc / 2), floorf(y - img.subtex->height * esc / 2), 0.0f, NULL, esc, esc);
}
static void dibujarFx(C2D_SpriteSheet hojaFx) {
    for (int i = 0; i < MAX_FX; i++) if (efectos[i].activo) {
        const Efecto* e = &efectos[i];
        dibujarImagenCentrada(hojaFx, e->base + e->t / e->dur, e->x, e->y, e->escala);
    }
}

// --- Numeros de dano flotantes ---
#define MAX_NUMEROS 4
typedef struct { bool activo; int valor, t; float x, y; u32 color; } NumeroDanio;
static NumeroDanio numeros[MAX_NUMEROS];
static void lanzarNumero(int valor, float x, float y, u32 color) {
    int libre = 0;
    for (int i = 0; i < MAX_NUMEROS; i++) if (!numeros[i].activo) { libre = i; break; }
    numeros[libre] = (NumeroDanio){true, valor, 0, x, y, color};
}
static void actualizarNumeros(void) {
    for (int i = 0; i < MAX_NUMEROS; i++) if (numeros[i].activo) {
        numeros[i].t++;
        numeros[i].y -= (numeros[i].t < 10) ? 2.0f : 0.3f;
        if (numeros[i].t > 45) numeros[i].activo = false;
    }
}
static void dibujarNumeros(C2D_TextBuf buf) {
    for (int i = 0; i < MAX_NUMEROS; i++) if (numeros[i].activo) {
        char s[8]; snprintf(s, sizeof(s), "%d", numeros[i].valor);
        float x = numeros[i].x, y = numeros[i].y;
        u32 negro = C2D_Color32(0, 0, 0, 255);
        dibujarTextoCentrado(buf, x - 1, y, 0.75f, negro, s);
        dibujarTextoCentrado(buf, x + 1, y, 0.75f, negro, s);
        dibujarTextoCentrado(buf, x, y - 1, 0.75f, negro, s);
        dibujarTextoCentrado(buf, x, y + 1, 0.75f, negro, s);
        dibujarTextoCentrado(buf, x, y, 0.75f, numeros[i].color, s);
    }
}

// Estado visual compartido del combate
typedef enum {
    F_MENU, F_AVANZA, F_ATACA, F_PROYECTIL, F_REGRESA, F_DEFENSA,
    F_ENE_AVANZA, F_ENE_GOLPE, F_ENE_REGRESA, F_VICTORIA, F_DERROTA
} FaseCombate;
static int eneFlash = 0, eneTemblor = 0, jugFlash = 0;

static void golpearEnemigo(Entidad* e, int dmg, float x, float y) {
    e->hp -= dmg; if (e->hp < 0) e->hp = 0;
    eneFlash = 8; eneTemblor = 12;
    lanzarFx(FX_IMPACTO_0, 3, 4, x, y, 0.0f);
    lanzarNumero(dmg, ENE_X + ENE_TAM / 2, ENE_Y + 10, C2D_Color32(255, 255, 255, 255));
}

// Acerca v a objetivo como maximo 'paso' (aceleracion/frenado lineal)
static float acercar(float v, float objetivo, float paso) {
    if (v < objetivo) return (v + paso > objetivo) ? objetivo : v + paso;
    return (v - paso < objetivo) ? objetivo : v - paso;
}

// Direccion a partir del D-Pad (dx, dy en -1..1). Devuelve -1 si no hay movimiento.
static int direccionDesde(int dx, int dy) {
    static const int tabla[3][3] = {
        {DIR_NO, DIR_N, DIR_NE},   // dy = -1 (arriba)
        {DIR_O,  -1,    DIR_E },   // dy =  0
        {DIR_SO, DIR_S, DIR_SE}    // dy = +1 (abajo)
    };
    return tabla[dy + 1][dx + 1];
}

#if USAR_PERSONALIZACION
static int ciclar(int valor, int delta, int total) { return (valor + delta + total) % total; }
#endif

// Marco con borde (estilo de la UI existente)
static void dibujarPanel(float x, float y, float w, float h, u32 borde, u32 fondo) {
    C2D_DrawRectSolid(x, y, 0.0f, w, h, borde);
    C2D_DrawRectSolid(x + 2, y + 2, 0.0f, w - 4, h - 4, fondo);
}

#if USAR_PERSONALIZACION
static void pedirNombre(char* destino, size_t tam) {
    SwkbdState swkbd;
    char buf[16];
    swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, 12);
    swkbdSetHintText(&swkbd, "Nombre del heroe");
    swkbdSetInitialText(&swkbd, destino);
    swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
    if (swkbdInputText(&swkbd, buf, sizeof(buf)) == SWKBD_BUTTON_CONFIRM) {
        strncpy(destino, buf, tam - 1);
        destino[tam - 1] = '\0';
    }
}
#endif

int main(int argc, char* argv[]) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    // Cargar la hoja de sprites del personaje y forzar filtrado "nearest" (pixel art nitido)
    C2D_SpriteSheet hojaHeroes = C2D_SpriteSheetLoadFromMem(heroes_t3x, heroes_t3x_size);
    if (!hojaHeroes) svcBreak(USERBREAK_PANIC);
    C3D_TexSetFilter(C2D_SpriteSheetGetImage(hojaHeroes, 0).tex, GPU_NEAREST, GPU_NEAREST);
    // Hojas de combate (una por viajero) y efectos
    const void* datosCombate[NUM_HEROES] = {combate_ophilia_t3x, combate_cyrus_t3x, combate_tressa_t3x, combate_olberic_t3x,
                                            combate_primrose_t3x, combate_alfyn_t3x, combate_therion_t3x, combate_haanit_t3x};
    const size_t tamCombate[NUM_HEROES] = {combate_ophilia_t3x_size, combate_cyrus_t3x_size, combate_tressa_t3x_size, combate_olberic_t3x_size,
                                           combate_primrose_t3x_size, combate_alfyn_t3x_size, combate_therion_t3x_size, combate_haanit_t3x_size};
    C2D_SpriteSheet hojasCombate[NUM_HEROES];
    for (int i = 0; i < NUM_HEROES; i++) {
        hojasCombate[i] = C2D_SpriteSheetLoadFromMem(datosCombate[i], tamCombate[i]);
        if (!hojasCombate[i]) svcBreak(USERBREAK_PANIC);
        C3D_TexSetFilter(C2D_SpriteSheetGetImage(hojasCombate[i], 0).tex, GPU_NEAREST, GPU_NEAREST);
    }
    C2D_SpriteSheet hojaFx = C2D_SpriteSheetLoadFromMem(fx_t3x, fx_t3x_size);
    if (!hojaFx) svcBreak(USERBREAK_PANIC);
    C3D_TexSetFilter(C2D_SpriteSheetGetImage(hojaFx, 0).tex, GPU_NEAREST, GPU_NEAREST);
#if USAR_PERSONALIZACION
    C2D_SpriteSheet hojaPJ = C2D_SpriteSheetLoadFromMem(personaje_t3x, personaje_t3x_size);
    if (!hojaPJ) svcBreak(USERBREAK_PANIC);
    C3D_TexSetFilter(C2D_SpriteSheetGetImage(hojaPJ, 0).tex, GPU_NEAREST, GPU_NEAREST);
#endif

    // Crear lienzos gráficos para AMBAS pantallas
    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    // Buffer para almacenar textos dinámicos de cada frame
    C2D_TextBuf dynamicBuf = C2D_TextBufNew(4096);

    srand(time(NULL));

    EstadoJuego estado = ESTADO_SELECCION;
    Entidad jugador = {"Heroe", 100, 100, 18, 5, false};
    Entidad enemigo = {"Goblin", 55, 55, 16, 3, false};

    float playerX = 200.0f, playerY = 150.0f; // posicion de los pies
    int contadorPasos = 0;
    int opcionMenu = 0; // 0: Atacar, 1: Defender, 2: Huir
    // Animacion del combate
    FaseCombate fase = F_MENU;
    int faseT = 0;
    float jugOffX = 0.0f, avanceMax = 0.0f, jugRetroceso = 0.0f, eneOffX = 0.0f;
    int danioPend = 0;
    bool proyActivo = false;
    float proyX = 0.0f, proyY = 0.0f;

    // Seleccion de viajero
    int heroeSel = 0;          // carta resaltada en la seleccion
    int heroe = 0;             // viajero elegido (-1 = personaje creado, si USAR_PERSONALIZACION)
    int dirSel = HDIR_ABAJO;   // direccion de la vista previa grande
    int esperaGiroSel = 0;
    const int dirsGiro[4] = {HDIR_ABAJO, HDIR_IZQ, HDIR_ARRIBA, HDIR_DER};
    int pasoGiro = 0;
    const float cartaW = 74.0f, cartaH = 92.0f, cartaX0 = 6.0f, cartaY0 = 34.0f, cartaSep = 4.0f;
#if USAR_PERSONALIZACION
    Apariencia apariencia = {0, 0, 0, 0, 0};
    int opcionPJ = 0;          // fila seleccionada en el menu de personalizacion
    int indiceNombre = 0;
#endif
    int contadorAnim = 0;      // reloj para animar la vista previa
    float velX = 0.0f, velY = 0.0f; // velocidad actual (con aceleracion)
    float distAnim = 0.0f;           // distancia recorrida en la caminata actual
    int dirObjetivo = DIR_S;         // direccion pedida con el D-Pad
    int framesGiro = 0;              // reloj del giro gradual
    bool caminando = false;
    int dirJugador = DIR_S;     // ultima direccion mirada (se conserva al detenerse)
#if USAR_PERSONALIZACION
    int dirPreview = DIR_S;     // direccion de la vista previa
    int esperaGiroManual = 0;   // tras L/R, pausa el giro automatico
#endif
    int dirCandidata = DIR_S;   // evita perder la diagonal al soltar los dos botones
    int framesCandidata = 0;
#if USAR_PERSONALIZACION
    enum { OPC_NOMBRE, OPC_PIEL, OPC_PEINADO, OPC_COLOR_PELO, OPC_ROPA, OPC_COLOR_ROPA, OPC_COMENZAR, NUM_OPC };
    const char* etiquetasPJ[NUM_OPC] = {"Nombre", "Piel", "Peinado", "Color de pelo", "Ropa", "Color de ropa", "COMENZAR"};
    const float filaY0 = 34.0f, filaAlto = 25.0f;
#endif

    char logLinea1[64] = "Explora el mapa con el D-Pad.";
    char logLinea2[64] = "Los enemigos acechan en la hierba...";

    // Paleta de colores para la UI y el juego
    u32 clrPasto      = C2D_Color32(34, 139, 34, 255);
    u32 clrArena      = C2D_Color32(45, 30, 55, 255);
    u32 clrVidaBg     = C2D_Color32(60, 20, 20, 255);
    u32 clrVida       = C2D_Color32(0, 220, 90, 255);

    u32 clrUIFondo    = C2D_Color32(18, 20, 32, 255);
    u32 clrUIPanel    = C2D_Color32(32, 38, 58, 255);
    u32 clrUIBorde    = C2D_Color32(90, 110, 160, 255);
    u32 clrBtnNormal  = C2D_Color32(45, 55, 80, 255);
    u32 clrBtnSelect  = C2D_Color32(230, 60, 60, 255);
    u32 clrBlanco     = C2D_Color32(255, 255, 255, 255);
    u32 clrGris       = C2D_Color32(170, 180, 200, 255);

    const char* nombresCmd[3] = {"ATACAR", "DEFENDER (-30% DMG)", "HUIR"};

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if (kDown & KEY_START) break;
        contadorAnim++;

        // ==========================================
        // LÓGICA: SELECCIÓN DE VIAJERO
        // ==========================================
        if (estado == ESTADO_SELECCION) {
            int antes = heroeSel;
            if (kDown & KEY_LEFT)  heroeSel = (heroeSel % 4 == 0) ? heroeSel + 3 : heroeSel - 1;
            if (kDown & KEY_RIGHT) heroeSel = (heroeSel % 4 == 3) ? heroeSel - 3 : heroeSel + 1;
            if (kDown & (KEY_UP | KEY_DOWN)) heroeSel = (heroeSel + 4) % NUM_HEROES;
            bool confirmar = (kDown & KEY_A);

            if (kDown & KEY_TOUCH) {
                touchPosition t;
                hidTouchRead(&t);
                for (int i = 0; i < NUM_HEROES; i++) {
                    float cx = cartaX0 + (i % 4) * (cartaW + cartaSep);
                    float cy = cartaY0 + (i / 4) * (cartaH + cartaSep);
                    if (t.px >= cx && t.px < cx + cartaW && t.py >= cy && t.py < cy + cartaH) {
                        if (i == heroeSel) confirmar = true; // tocar la carta ya elegida = comenzar
                        heroeSel = i;
                    }
                }
            }
            if (heroeSel != antes) { pasoGiro = 0; dirSel = HDIR_ABAJO; esperaGiroSel = 0; contadorAnim = 0; }

            // L/R giran la vista previa; si no se tocan, gira sola cada 3 ciclos de paso
            if (kDown & KEY_L) { pasoGiro = (pasoGiro + 3) % 4; dirSel = dirsGiro[pasoGiro]; esperaGiroSel = 240; }
            if (kDown & KEY_R) { pasoGiro = (pasoGiro + 1) % 4; dirSel = dirsGiro[pasoGiro]; esperaGiroSel = 240; }
            if (esperaGiroSel > 0) esperaGiroSel--;
            else if (contadorAnim % 96 == 95) { pasoGiro = (pasoGiro + 1) % 4; dirSel = dirsGiro[pasoGiro]; }

#if USAR_PERSONALIZACION
            if (kDown & KEY_X) { estado = ESTADO_PERSONALIZACION; opcionPJ = 0; }
#endif
            if (confirmar) {
                const Heroe* he = &HEROES[heroeSel];
                heroe = heroeSel;
                snprintf(jugador.nombre, sizeof(jugador.nombre), "%s", he->nombre);
                jugador.maxHp = jugador.hp = he->hp;
                jugador.atk = he->atk;
                jugador.def = he->def;
                playerX = 200.0f; playerY = 150.0f;
                velX = velY = 0.0f; caminando = false; distAnim = 0.0f;
                dirJugador = dirObjetivo = DIR_S;
                estado = ESTADO_EXPLORACION;
                snprintf(logLinea1, sizeof(logLinea1), "¡Bienvenido, %s!", jugador.nombre);
                snprintf(logLinea2, sizeof(logLinea2), "Explora el mapa con el D-Pad.");
            }
        }
#if USAR_PERSONALIZACION
        // ==========================================
        // LÓGICA: CREADOR DE PERSONAJE (oculto)
        // ==========================================
        else if (estado == ESTADO_PERSONALIZACION) {
            if (kDown & KEY_UP)   opcionPJ = ciclar(opcionPJ, -1, NUM_OPC);
            if (kDown & KEY_DOWN) opcionPJ = ciclar(opcionPJ, +1, NUM_OPC);

            int delta = 0;
            if (kDown & KEY_LEFT)  delta = -1;
            if (kDown & KEY_RIGHT) delta = +1;
            bool aceptar = (kDown & KEY_A);

            // Pantalla tactil: tocar una fila la selecciona; las flechas cambian el valor
            if (kDown & KEY_TOUCH) {
                touchPosition t;
                hidTouchRead(&t);
                int fila = (int)((t.py - filaY0) / filaAlto);
                if (t.py >= filaY0 && fila >= 0 && fila < NUM_OPC && t.px >= 8 && t.px <= 312) {
                    opcionPJ = fila;
                    if (fila == OPC_COMENZAR || (fila == OPC_NOMBRE && t.px > 185 && t.px < 270)) aceptar = true;
                    else if (t.px >= 140 && t.px < 185) delta = -1;
                    else if (t.px >= 270) delta = +1;
                }
            }

            // L/R giran la vista previa; si no se tocan, gira sola
            if (kDown & KEY_L) { dirPreview = ciclar(dirPreview, -1, NUM_DIRS); esperaGiroManual = 180; }
            if (kDown & KEY_R) { dirPreview = ciclar(dirPreview, +1, NUM_DIRS); esperaGiroManual = 180; }
            if (esperaGiroManual > 0) esperaGiroManual--;
            else if (contadorAnim % 64 == 0) dirPreview = ciclar(dirPreview, +1, NUM_DIRS);

            if (kDown & KEY_X) { // Aleatorio
                apariencia.piel      = rand() % NUM_PIELES;
                apariencia.peinado   = rand() % NUM_PEINADOS;
                apariencia.colorPelo = rand() % NUM_COLORES_PELO;
                apariencia.ropa      = rand() % NUM_ROPAS;
                apariencia.colorRopa = rand() % NUM_COLORES_ROPA;
            }

            if (delta != 0) {
                switch (opcionPJ) {
                    case OPC_NOMBRE:
                        indiceNombre = ciclar(indiceNombre, delta, NUM_NOMBRES);
                        snprintf(jugador.nombre, sizeof(jugador.nombre), "%s", NOMBRES[indiceNombre]);
                        break;
                    case OPC_PIEL:       apariencia.piel      = ciclar(apariencia.piel, delta, NUM_PIELES); break;
                    case OPC_PEINADO:    apariencia.peinado   = ciclar(apariencia.peinado, delta, NUM_PEINADOS); break;
                    case OPC_COLOR_PELO: apariencia.colorPelo = ciclar(apariencia.colorPelo, delta, NUM_COLORES_PELO); break;
                    case OPC_ROPA:       apariencia.ropa      = ciclar(apariencia.ropa, delta, NUM_ROPAS); break;
                    case OPC_COLOR_ROPA: apariencia.colorRopa = ciclar(apariencia.colorRopa, delta, NUM_COLORES_ROPA); break;
                }
            }

            if (aceptar) {
                if (opcionPJ == OPC_NOMBRE) {
                    pedirNombre(jugador.nombre, sizeof(jugador.nombre));
                } else if (opcionPJ == OPC_COMENZAR) {
                    estado = ESTADO_EXPLORACION;
                    heroe = -1;
                    jugador.maxHp = jugador.hp = 100; jugador.atk = 18; jugador.def = 5;
                    playerX = 200.0f; playerY = 150.0f;
                    snprintf(logLinea1, sizeof(logLinea1), "¡Bienvenido, %s!", jugador.nombre);
                    snprintf(logLinea2, sizeof(logLinea2), "Explora el mapa con el D-Pad.");
                }
            }
        }
#endif
        // ==========================================
        // LÓGICA: MODO EXPLORACIÓN
        // ==========================================
        else if (estado == ESTADO_EXPLORACION) {
            if (kDown & KEY_SELECT) { estado = ESTADO_SELECCION; velX = velY = 0.0f; caminando = false; if (heroe >= 0) heroeSel = heroe; }
            int dx = ((kHeld & KEY_RIGHT) ? 1 : 0) - ((kHeld & KEY_LEFT) ? 1 : 0);
            int dy = ((kHeld & KEY_DOWN)  ? 1 : 0) - ((kHeld & KEY_UP)   ? 1 : 0);
            float objX = 0.0f, objY = 0.0f;
            if (dx != 0 || dy != 0) {
                // Direccion pedida: al instante si estabas quieto; si ya caminabas, debe
                // mantenerse 4 frames (al soltar una diagonal los dos botones no se sueltan a la vez).
                int dirNueva = direccionDesde(dx, dy);
                if (!caminando) { if (dirNueva != dirObjetivo) framesGiro = FRAMES_X_GIRO - 1; dirObjetivo = dirNueva; framesCandidata = 0; }
                else if (dirNueva != dirObjetivo) {
                    if (dirNueva == dirCandidata) framesCandidata++;
                    else { dirCandidata = dirNueva; framesCandidata = 1; }
                    if (framesCandidata >= 4) { dirObjetivo = dirNueva; framesCandidata = 0; }
                } else framesCandidata = 0;
                float n = (dx != 0 && dy != 0) ? 0.7071f : 1.0f; // diagonal normalizada
                objX = dx * n * VEL_MAX;
                objY = dy * n * VEL_MAX;
            }
            // Aceleracion / frenado suaves
            velX = acercar(velX, objX, (objX != 0.0f) ? ACELERACION : FRENADO);
            velY = acercar(velY, objY, (objY != 0.0f) ? ACELERACION : FRENADO);
            float nx = playerX + velX, ny = playerY + velY;
            // Limites de la pantalla segun el tamano del sprite (x, y = pies)
            float limIzq = 16.0f, limDer = 16.0f, limArr = 30.0f;
            if (heroe >= 0) {
                const Heroe* he = &HEROES[heroe];
                limIzq = he->anclaX; limDer = he->ancho - he->anclaX; limArr = he->anclaY;
            }
            if (nx < limIzq) { nx = limIzq; velX = 0; }
            if (nx > 400 - limDer) { nx = 400 - limDer; velX = 0; }
            if (ny < limArr) { ny = limArr; velY = 0; }
            if (ny > 238) { ny = 238; velY = 0; }
            float movido = sqrtf((nx - playerX) * (nx - playerX) + (ny - playerY) * (ny - playerY));
            playerX = nx; playerY = ny;
            bool seMovio = movido > 0.05f;
            caminando = seMovio;
            if (seMovio) distAnim += movido; else distAnim = 0.0f;

            // Giro gradual: rota de 45 en 45 grados por el camino mas corto (S -> SE -> E)
            if (dirJugador != dirObjetivo) {
                if (++framesGiro >= FRAMES_X_GIRO) {
                    framesGiro = 0;
                    int dif = (dirObjetivo - dirJugador + NUM_DIRS) % NUM_DIRS;
                    dirJugador = (dirJugador + (dif <= NUM_DIRS / 2 ? 1 : NUM_DIRS - 1)) % NUM_DIRS;
                }
            } else framesGiro = 0;

            bool forzarCombate = (kDown & KEY_Y);  // Y: combate de prueba
            if (forzarCombate) { seMovio = true; contadorPasos = 29; }
            if (seMovio) {
                contadorPasos++;
                if (contadorPasos >= 30) {
                    contadorPasos = 0;
                    if ((rand() % 100) < 10 || forzarCombate) {
                        estado = ESTADO_COMBATE;
                        velX = velY = 0.0f;
                        enemigo.maxHp = 45 + (rand() % 25);
                        enemigo.hp = enemigo.maxHp;
                        opcionMenu = 0;
                        fase = F_MENU; faseT = 0; jugOffX = eneOffX = jugRetroceso = 0.0f;
                        proyActivo = false; eneFlash = eneTemblor = jugFlash = 0;
                        memset(efectos, 0, sizeof(efectos)); memset(numeros, 0, sizeof(numeros));
                        snprintf(logLinea1, sizeof(logLinea1), "¡Un %s salvaje corta el paso!", enemigo.nombre);
                        snprintf(logLinea2, sizeof(logLinea2), "Selecciona un comando de accion.");
                    }
                }
            }
        }
        // ==========================================
        // LÓGICA: MODO COMBATE
        // ==========================================
        else if (estado == ESTADO_COMBATE) {
            faseT++;
            actualizarFx();
            actualizarNumeros();
            if (jugFlash > 0) jugFlash--;
            if (eneFlash > 0) eneFlash--;
            if (eneTemblor > 0) eneTemblor--;
            jugRetroceso *= 0.85f;
            EstiloAtaque estilo = (heroe >= 0) ? ESTILO_HEROE[heroe] : EST_TAJO;
            float eneCX = ENE_X + ENE_TAM / 2 + eneOffX, eneCY = ENE_Y + ENE_TAM / 2;

            if (fase == F_MENU) {
                if (kDown & KEY_UP)   opcionMenu = (opcionMenu + 2) % 3;
                if (kDown & KEY_DOWN) opcionMenu = (opcionMenu + 1) % 3;

                bool confirmarAccion = (kDown & KEY_A);

                // Soporte para Pantalla Táctil en los 3 botones de comando
                if (kDown & KEY_TOUCH) {
                    touchPosition touch;
                    hidTouchRead(&touch);
                    for (int i = 0; i < 3; i++) {
                        int btnY = 128 + (i * 34);
                        if (touch.px >= 20 && touch.px <= 300 && touch.py >= btnY && touch.py <= btnY + 28) {
                            opcionMenu = i;
                            confirmarAccion = true;
                        }
                    }
                }

                if (confirmarAccion) {
                    jugador.defendiendo = false;
                    faseT = 0;
                    if (opcionMenu == 0) { // ATACAR: el dano se aplica cuando el arma conecta
                        danioPend = jugador.atk - enemigo.def + (rand() % 5);
                        if (danioPend < 1) danioPend = 1;
                        snprintf(logLinea1, sizeof(logLinea1), "¡%s ataca!", jugador.nombre);
                        logLinea2[0] = '\0';
                        if (esCuerpoACuerpo(estilo)) {
                            // corre hasta que la punta del arma (cuadro de golpe) toque al enemigo
                            int alc = (heroe >= 0) ? META_COMBATE[heroe].alcance[CUADRO_GOLPE][0] : 20;
                            avanceMax = ENE_X + 6.0f - JUG_X - alc * ESCALA_COMBATE;
                            if (avanceMax < 0) avanceMax = 0;
                            fase = F_AVANZA;
                        } else fase = F_ATACA;
                    }
                    else if (opcionMenu == 1) { // DEFENDER (-30% daño recibido)
                        jugador.defendiendo = true;
                        snprintf(logLinea1, sizeof(logLinea1), "Adoptas postura defensiva (-30%% daño).");
                        logLinea2[0] = '\0';
                        fase = F_DEFENSA;
                    }
                    else if (opcionMenu == 2) { // HUIR
                        if ((rand() % 100) < 65) {
                            estado = ESTADO_EXPLORACION;
                            snprintf(logLinea1, sizeof(logLinea1), "¡Escapaste del combate con exito!");
                            snprintf(logLinea2, sizeof(logLinea2), "Continua explorando el mapa.");
                        } else {
                            snprintf(logLinea1, sizeof(logLinea1), "¡Intentaste huir pero el enemigo te bloqueo!");
                            logLinea2[0] = '\0';
                            fase = F_ENE_AVANZA;
                        }
                    }
                }
            }
            else if (fase == F_AVANZA) {
                jugOffX = avanceMax * suave(faseT / (float)FR_AVANCE);
                if (faseT >= FR_AVANCE) { fase = F_ATACA; faseT = 0; }
            }
            else if (fase == F_ATACA) {
                int tGolpe = inicioCuadroAtaque(CUADRO_GOLPE);
                int tSegundo = inicioCuadroAtaque(5);
                bool dosGolpes = (estilo == EST_DOBLE || estilo == EST_GIRO);
                int d1 = dosGolpes ? (danioPend + 1) / 2 : danioPend;
                const MetaCombate* mc = &META_COMBATE[heroe >= 0 ? heroe : 0];
                float puntaX = JUG_X + jugOffX + mc->alcance[CUADRO_GOLPE][0] * ESCALA_COMBATE;
                float puntaY = JUG_Y + mc->alcance[CUADRO_GOLPE][1] * ESCALA_COMBATE;
                if (faseT == tGolpe) {
                    switch (estilo) {
                        case EST_TAJO: case EST_HACHA: case EST_DOBLE: case EST_GIRO:
                            lanzarFx(FX_TAJO_0, 3, 4, eneCX - 14, eneCY, 0.0f);
                            golpearEnemigo(&enemigo, d1, eneCX, eneCY);
                            break;
                        case EST_ESTOCADA:
                            lanzarFx(FX_ESTOCADA_0, 2, 5, eneCX - 30, puntaY, 1.5f);
                            golpearEnemigo(&enemigo, d1, eneCX - 6, puntaY);
                            break;
                        case EST_FLECHA: case EST_ORBE:
                            proyActivo = true; proyX = puntaX; proyY = puntaY;
                            break;
                        case EST_LUZ:
                            lanzarFx(FX_LUZ_0, 4, 5, eneCX, eneCY, 0.0f);
                            lanzarFx(FX_BRILLO_0, 3, 5, puntaX, puntaY - 10, 0.0f);
                            break;
                    }
                    if (estilo != EST_FLECHA && estilo != EST_ORBE && estilo != EST_LUZ)
                        snprintf(logLinea1, sizeof(logLinea1), "¡Atacas al %s! Causas %d DMG.", enemigo.nombre, danioPend);
                }
                if (estilo == EST_LUZ && faseT == tGolpe + 8) {
                    golpearEnemigo(&enemigo, danioPend, eneCX, eneCY);
                    snprintf(logLinea1, sizeof(logLinea1), "¡La luz sagrada golpea! %d DMG.", danioPend);
                }
                if (dosGolpes && faseT == tSegundo && danioPend - d1 > 0) {
                    lanzarFx(FX_TAJO_0, 3, 4, eneCX - 8, eneCY - 10, 0.0f);
                    golpearEnemigo(&enemigo, danioPend - d1, eneCX + 6, eneCY - 8);
                }
                if (faseT >= inicioCuadroAtaque(6) && !(estilo == EST_LUZ && faseT < tGolpe + 9)) {
                    faseT = 0;
                    fase = esCuerpoACuerpo(estilo) ? F_REGRESA : F_PROYECTIL;
                }
            }
            else if (fase == F_REGRESA) {
                jugOffX = avanceMax * (1.0f - suave(faseT / (float)FR_AVANCE));
                if (faseT >= FR_AVANCE) { jugOffX = 0; faseT = 0; fase = (enemigo.hp <= 0) ? F_VICTORIA : F_ENE_AVANZA; }
            }
            else if (fase == F_PROYECTIL) {
                // espera a que la flecha/orbe llegue y deja respirar el impacto
                if (!proyActivo && faseT > 14) { faseT = 0; fase = (enemigo.hp <= 0) ? F_VICTORIA : F_ENE_AVANZA; }
                else if (proyActivo) faseT = 0;
            }
            else if (fase == F_DEFENSA) {
                if (faseT >= 20) { faseT = 0; fase = F_ENE_AVANZA; }
            }
            else if (fase == F_ENE_AVANZA) {
                eneOffX = -130.0f * suave(faseT / (float)FR_ENE_AVANCE);
                if (faseT >= FR_ENE_AVANCE) {
                    faseT = 0; fase = F_ENE_GOLPE;
                    int dmgBruto = enemigo.atk - jugador.def + (rand() % 4);
                    if (dmgBruto < 1) dmgBruto = 1;
                    int dmgFinal = dmgBruto;
                    if (jugador.defendiendo) {
                        dmgFinal = (int)(dmgBruto * 0.70f);
                        if (dmgFinal < 1) dmgFinal = 1;
                    }
                    jugador.hp -= dmgFinal;
                    if (jugador.hp < 0) jugador.hp = 0;
                    lanzarFx(FX_GARRA_0, 2, 5, JUG_X + 14, JUG_Y - 60, 0.0f);
                    lanzarNumero(dmgFinal, JUG_X + 56, JUG_Y - 146, jugador.defendiendo ? C2D_Color32(140, 230, 255, 255) : C2D_Color32(255, 110, 100, 255));
                    if (jugador.defendiendo) {
                        jugRetroceso = -4.0f;
                        lanzarFx(FX_IMPACTO_0, 3, 3, JUG_X + 46, JUG_Y - 64, 0.0f);
                        snprintf(logLinea2, sizeof(logLinea2), "%s ataca: ¡Bloqueas! Recibes %d DMG.", enemigo.nombre, dmgFinal);
                    } else {
                        jugRetroceso = -14.0f; jugFlash = 12;
                        snprintf(logLinea2, sizeof(logLinea2), "¡%s contraataca! Recibes %d DMG.", enemigo.nombre, dmgFinal);
                    }
                }
            }
            else if (fase == F_ENE_GOLPE) {
                if (faseT >= FR_ENE_GOLPE) { faseT = 0; fase = F_ENE_REGRESA; }
            }
            else if (fase == F_ENE_REGRESA) {
                eneOffX = -130.0f * (1.0f - suave(faseT / (float)FR_ENE_AVANCE));
                if (faseT >= FR_ENE_AVANCE) {
                    eneOffX = 0; faseT = 0;
                    if (jugador.hp <= 0) {
                        fase = F_DERROTA;
                        snprintf(logLinea1, sizeof(logLinea1), "Has sido derrotado...");
                        snprintf(logLinea2, sizeof(logLinea2), "Te recuperas en la zona segura.");
                    } else fase = F_MENU;
                }
            }
            else if (fase == F_VICTORIA) {
                if (faseT == 1) {
                    snprintf(logLinea1, sizeof(logLinea1), "¡Victoria! Derrotaste al %s.", enemigo.nombre);
                    snprintf(logLinea2, sizeof(logLinea2), "Explora el mapa para buscar mas combates.");
                }
                if (faseT % 18 == 10 && faseT > 30 && faseT < 110)
                    lanzarFx(FX_BRILLO_0, 3, 6, JUG_X - 30 + (rand() % 60), JUG_Y - 40 - (rand() % 80), 0.0f);
                if (faseT >= FR_VICTORIA) { estado = ESTADO_EXPLORACION; fase = F_MENU; }
            }
            else if (fase == F_DERROTA) {
                if (faseT >= FR_DERROTA) {
                    jugador.hp = jugador.maxHp;
                    estado = ESTADO_EXPLORACION;
                    fase = F_MENU;
                }
            }

            // Proyectil (flecha de H'aanit / orbe de Cyrus)
            if (proyActivo) {
                proyX += (estilo == EST_FLECHA) ? 9.0f : 6.0f;
                if (proyX >= eneCX - 18) {
                    proyActivo = false;
                    golpearEnemigo(&enemigo, danioPend, eneCX - 10, proyY);
                    if (estilo == EST_ORBE) lanzarFx(FX_LUZ_1, 3, 4, eneCX - 6, proyY, 0.0f);
                    snprintf(logLinea1, sizeof(logLinea1), "¡Atacas al %s! Causas %d DMG.", enemigo.nombre, danioPend);
                }
            }
        }

        // ==========================================
        // RENDERIZADO GRÁFICO (AMBAS PANTALLAS)
        // ==========================================
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TextBufClear(dynamicBuf); // Limpiar caché de texto del frame anterior

        // --- 1. PANTALLA SUPERIOR (400x240) ---
        C2D_SceneBegin(top);
        if (estado == ESTADO_SELECCION) {
            const Heroe* he = &HEROES[heroeSel];
            C2D_TargetClear(top, clrUIFondo);
            dibujarPanel(100.0f, 6.0f, 200.0f, 26.0f, clrUIBorde, clrUIPanel);
            dibujarTextoCentrado(dynamicBuf, 200.0f, 10.0f, 0.55f, clrBlanco, "ELIGE A TU VIAJERO");

            // Vista previa grande (2x = 128 px de alto), caminando en el sitio
            dibujarPanel(12.0f, 38.0f, 186.0f, 194.0f, clrUIBorde, clrUIPanel);
            C2D_DrawRectSolid(16.0f, 42.0f, 0.0f, 178.0f, 186.0f, C2D_Color32(44, 52, 78, 255));
            C2D_DrawRectSolid(16.0f, 188.0f, 0.0f, 178.0f, 40.0f, C2D_Color32(38, 44, 66, 255));
            dibujarSombra(105.0f, 206.0f, he->anchoCuerpo * 2.0f, C2D_Color32(24, 28, 44, 255));
            dibujarHeroe(hojaHeroes, heroeSel, dirSel, (contadorAnim / 8) % HEROE_CUADROS, 105.0f, 206.0f, 2.0f);

            // Ficha del viajero
            dibujarPanel(206.0f, 38.0f, 182.0f, 194.0f, clrUIBorde, clrUIPanel);
            dibujarTexto(dynamicBuf, 216.0f, 44.0f, 0.75f, clrBlanco, he->nombre);
            C2D_DrawRectSolid(216.0f, 72.0f, 0.0f, 162.0f, 2.0f, clrBtnSelect);
            dibujarTexto(dynamicBuf, 216.0f, 78.0f, 0.5f, clrGris, he->clase);
            dibujarTexto(dynamicBuf, 216.0f, 98.0f, 0.4f, clrBlanco, he->lema);
            const char* etq[3] = {"HP", "ATK", "DEF"};
            int val[3] = {he->hp, he->atk, he->def};
            int maxv[3] = {130, 22, 10};
            for (int i = 0; i < 3; i++) {
                float sy = 124.0f + i * 20.0f;
                char num[8];
                snprintf(num, sizeof(num), "%d", val[i]);
                dibujarTexto(dynamicBuf, 216.0f, sy, 0.42f, clrGris, etq[i]);
                dibujarBarra(252.0f, sy + 4, 96.0f, 8.0f, (float)val[i] / maxv[i], clrVidaBg, i == 0 ? clrVida : C2D_Color32(230, 190, 60, 255));
                dibujarTexto(dynamicBuf, 354.0f, sy, 0.42f, clrBlanco, num);
            }
            dibujarTexto(dynamicBuf, 216.0f, 192.0f, 0.38f, clrGris, "L/R: girar   A: comenzar");
            dibujarTexto(dynamicBuf, 216.0f, 208.0f, 0.38f, clrGris, USAR_PERSONALIZACION ? "X: crear personaje" : "START: salir");
        }
#if USAR_PERSONALIZACION
        else if (estado == ESTADO_PERSONALIZACION) {
            C2D_TargetClear(top, clrUIFondo);
            // Titulo
            dibujarPanel(100.0f, 6.0f, 200.0f, 26.0f, clrUIBorde, clrUIPanel);
            dibujarTexto(dynamicBuf, 118.0f, 10.0f, 0.55f, clrBlanco, "CREA TU PERSONAJE");

            // Panel principal con vista previa grande (4x = 128x128)
            dibujarPanel(110.0f, 38.0f, 180.0f, 178.0f, clrUIBorde, clrUIPanel);
            C2D_DrawRectSolid(114.0f, 42.0f, 0.0f, 172.0f, 150.0f, C2D_Color32(44, 52, 78, 255));
            C2D_DrawEllipseSolid(150.0f, 166.0f, 0.0f, 100.0f, 18.0f, C2D_Color32(24, 28, 44, 255));
            int cuadroPrev = cuadroPorDistancia(contadorAnim * VEL_MAX); // misma cadencia que en el mapa
            dibujarPersonaje(hojaPJ, &apariencia, dirPreview, cuadroPrev, 136.0f, 52.0f, 4.0f);
            dibujarTexto(dynamicBuf, 120.0f, 194.0f, 0.5f, clrBlanco, jugador.nombre);

            // Vista lateral: tamano real en el mapa (1x y 2x)
            dibujarPanel(300.0f, 38.0f, 92.0f, 122.0f, clrUIBorde, clrUIPanel);
            dibujarTexto(dynamicBuf, 310.0f, 42.0f, 0.4f, clrGris, "En el mapa");
            C2D_DrawRectSolid(304.0f, 60.0f, 0.0f, 84.0f, 96.0f, clrPasto);
            dibujarPersonaje(hojaPJ, &apariencia, dirPreview, CUADRO_QUIETO, 314.0f, 82.0f, 1.0f);
            dibujarPersonaje(hojaPJ, &apariencia, dirPreview, cuadroPrev, 330.0f, 76.0f, 2.0f);

            // Resumen a la izquierda
            dibujarPanel(8.0f, 38.0f, 94.0f, 178.0f, clrUIBorde, clrUIPanel);
            dibujarTexto(dynamicBuf, 16.0f, 44.0f, 0.42f, clrGris, "Estilo");
            dibujarTexto(dynamicBuf, 16.0f, 62.0f, 0.42f, clrBlanco, PEINADOS[apariencia.peinado]);
            dibujarTexto(dynamicBuf, 16.0f, 80.0f, 0.42f, clrBlanco, ROPAS[apariencia.ropa]);
            dibujarTexto(dynamicBuf, 16.0f, 106.0f, 0.42f, clrGris, "Colores");
            const ColorNombrado* muestras[3] = {&PIELES[apariencia.piel], &COLORES_PELO[apariencia.colorPelo], &COLORES_ROPA[apariencia.colorRopa]};
            for (int i = 0; i < 3; i++) {
                float my = 126.0f + i * 22.0f;
                C2D_DrawRectSolid(16.0f, my, 0.0f, 16.0f, 16.0f, clrBlanco);
                C2D_DrawRectSolid(17.0f, my + 1, 0.0f, 14.0f, 14.0f, colorDe(muestras[i]));
                dibujarTexto(dynamicBuf, 38.0f, my + 1, 0.4f, clrBlanco, muestras[i]->nombre);
            }
            dibujarTexto(dynamicBuf, 300.0f, 166.0f, 0.38f, clrGris, "L/R: girar");
            dibujarTexto(dynamicBuf, 300.0f, 182.0f, 0.38f, clrGris, "X: aleatorio");
            dibujarTexto(dynamicBuf, 300.0f, 198.0f, 0.38f, clrGris, "START: salir");
        }
#endif
        else if (estado == ESTADO_EXPLORACION) {
            C2D_TargetClear(top, clrPasto);
            // Sombra bajo los pies + personaje personalizado (32x32, colision igual al tamano del sprite)
            float px = floorf(playerX + 0.5f), py = floorf(playerY + 0.5f);
            if (heroe >= 0) {
                dibujarSombra(px, py, HEROES[heroe].anchoCuerpo, C2D_Color32(20, 90, 20, 255));
                dibujarHeroe(hojaHeroes, heroe, dirHeroe(dirJugador), cuadroHeroe(caminando, distAnim), px, py, 1.0f);
            }
#if USAR_PERSONALIZACION
            else {
                C2D_DrawEllipseSolid(px - 8.0f, py - 3.0f, 0.0f, 16.0f, 5.0f, C2D_Color32(20, 90, 20, 255));
                dibujarPersonaje(hojaPJ, &apariencia, dirJugador, caminando ? cuadroPorDistancia(distAnim) : CUADRO_QUIETO, px - 16.0f, py - 30.0f, 1.0f);
            }
#endif
        } else {
            C2D_TargetClear(top, clrArena);
            C2D_SetTintMode(C2D_TintSolid);
            float jx = JUG_X + jugOffX + jugRetroceso;
            // Jugador: viajero elegido a 2x, de perfil mirando al enemigo, animado segun la fase
            if (heroe >= 0) {
                int c = CC_IDLE + (contadorAnim / 12) % 4;
                switch (fase) {
                    case F_AVANZA:   c = CC_ATAQUE; break;
                    case F_ATACA:    c = CC_ATAQUE + cuadroAtaque(faseT); break;
                    case F_PROYECTIL: c = (faseT < 8 || proyActivo) ? CC_ATAQUE + 5 : c; break;
                    case F_DEFENSA:  c = CC_DEFENSA + (faseT < 6 ? 0 : 1); break;
                    case F_ENE_AVANZA: case F_ENE_REGRESA:
                        if (jugador.defendiendo) c = CC_DEFENSA + 1;
                        else if (fase == F_ENE_REGRESA && faseT < 6) c = CC_HERIDO + 1;
                        break;
                    case F_ENE_GOLPE:
                        c = jugador.defendiendo ? CC_DEFENSA + 1 : CC_HERIDO + (faseT < 9 ? 0 : 1);
                        if (jugador.hp <= 0 && faseT > 9) c = CC_KO;
                        break;
                    case F_VICTORIA: c = (faseT < 24) ? c : CC_VICTORIA + ((faseT - 24) / 20) % 2; break;
                    case F_DERROTA:  c = (faseT < 26) ? CC_KO : CC_KO + 1; break;
                    default: break;
                }
                if (fase == F_ENE_REGRESA && jugador.hp <= 0) c = CC_KO;
                dibujarSombra(jx, JUG_Y, HEROES[heroe].anchoCuerpo * 2.0f, C2D_Color32(30, 20, 38, 255));
                C2D_ImageTint tinte;
                bool destello = (jugFlash > 0 && (jugFlash / 2) % 2 == 0);
                C2D_PlainImageTint(&tinte, destello ? C2D_Color32(255, 255, 255, 255) : C2D_Color32(255, 70, 70, 255), destello ? 0.85f : 0.35f);
                dibujarHeroeCombate(hojasCombate[heroe], heroe, c, jx, JUG_Y, ESCALA_COMBATE, jugFlash > 0 ? &tinte : NULL);
                dibujarTextoCentrado(dynamicBuf, JUG_X, 46.0f, 0.5f, clrBlanco, jugador.nombre);
            }
#if USAR_PERSONALIZACION
            else {
                C2D_DrawEllipseSolid(70.0f + jugOffX, 150.0f, 0.0f, 44.0f, 10.0f, C2D_Color32(30, 20, 38, 255));
                dibujarPersonaje(hojaPJ, &apariencia, DIR_E, CUADRO_QUIETO, 60.0f + jx - JUG_X, 96.0f, 2.0f);
            }
#endif
            if (jugador.defendiendo && fase >= F_DEFENSA && fase <= F_ENE_REGRESA) {
                // Escudo de luz celeste delante del viajero
                dibujarImagenCentrada(hojaFx, FX_ESCUDO_0 + (contadorAnim / 6) % 2, jx + 44.0f, JUG_Y - 62.0f, ESCALA_COMBATE);
            }

            // Enemigo (cuadro rojo provisional): embestida, temblor, destello y desvanecido al morir
            int alfa = 255;
            if (fase == F_VICTORIA) alfa = (faseT >= 30) ? 0 : 255 - faseT * 255 / 30;
            if (alfa > 0) {
                float tx = (eneTemblor > 0) ? ((eneTemblor % 4 < 2) ? 3.0f : -3.0f) : 0.0f;
                float ex = ENE_X + eneOffX + tx;
                float salto = (fase == F_ENE_AVANZA) ? -sinf(suave(faseT / (float)FR_ENE_AVANCE) * 3.14159f) * 18.0f : 0.0f;
                C2D_DrawEllipseSolid(ENE_X + eneOffX - 4.0f, ENE_Y + ENE_TAM - 5.0f, 0.0f, ENE_TAM + 8.0f, 12.0f, C2D_Color32(30, 20, 38, alfa));
                u32 cuerpo = (eneFlash > 0 && (eneFlash / 2) % 2 == 0) ? C2D_Color32(255, 255, 255, alfa) : C2D_Color32(220, 50, 50, alfa);
                C2D_DrawRectSolid(ex, ENE_Y + salto, 0.0f, ENE_TAM, ENE_TAM, cuerpo);
                u32 ojo = C2D_Color32(255, 230, 120, alfa);
                C2D_DrawRectSolid(ex + 9.0f, ENE_Y + salto + 16.0f, 0.0f, 8.0f, 6.0f, ojo);
                C2D_DrawRectSolid(ex + 24.0f, ENE_Y + salto + 16.0f, 0.0f, 8.0f, 6.0f, ojo);
                C2D_DrawRectSolid(270.0f, 78.0f, 0.0f, 65.0f, 7.0f, clrVidaBg);
                C2D_DrawRectSolid(270.0f, 78.0f, 0.0f, 65.0f * ((float)enemigo.hp / enemigo.maxHp), 7.0f, clrVida);
                char infoEnemigo[32];
                snprintf(infoEnemigo, sizeof(infoEnemigo), "%s (%d/%d)", enemigo.nombre, enemigo.hp, enemigo.maxHp);
                dibujarTexto(dynamicBuf, 255.0f, 58.0f, 0.45f, clrBlanco, infoEnemigo);
            }

            // Proyectil + efectos + numeros
            if (proyActivo) {
                if (heroe >= 0 && ESTILO_HEROE[heroe] == EST_ORBE)
                    dibujarImagenCentrada(hojaFx, FX_ORBE_0 + (contadorAnim / 4) % 2, proyX, proyY, ESCALA_COMBATE);
                else
                    dibujarImagenCentrada(hojaFx, FX_FLECHA, proyX, proyY, ESCALA_COMBATE);
            }
            dibujarFx(hojaFx);
            dibujarNumeros(dynamicBuf);
        }

        // --- 2. PANTALLA INFERIOR (UI Estilizada 320x240) ---
        C2D_SceneBegin(bottom);
        C2D_TargetClear(bottom, clrUIFondo);

        if (estado == ESTADO_SELECCION) {
            dibujarPanel(8.0f, 4.0f, 304.0f, 26.0f, clrUIBorde, clrUIPanel);
            dibujarTexto(dynamicBuf, 18.0f, 8.0f, 0.5f, clrBlanco, "Viajeros");
            char cuenta[16];
            snprintf(cuenta, sizeof(cuenta), "%d / %d", heroeSel + 1, NUM_HEROES);
            dibujarTexto(dynamicBuf, 258.0f, 9.0f, 0.45f, clrGris, cuenta);
            for (int i = 0; i < NUM_HEROES; i++) {
                float cx = cartaX0 + (i % 4) * (cartaW + cartaSep);
                float cy = cartaY0 + (i / 4) * (cartaH + cartaSep);
                bool sel = (i == heroeSel);
                dibujarPanel(cx, cy, cartaW, cartaH, sel ? clrBlanco : clrUIBorde, sel ? C2D_Color32(58, 70, 104, 255) : clrUIPanel);
                C2D_DrawRectSolid(cx + 2, cy + cartaH - 20, 0.0f, cartaW - 4, 18.0f, sel ? clrBtnSelect : clrBtnNormal);
                float pieX = cx + cartaW / 2, pieY = cy + 70.0f;
                dibujarSombra(pieX, pieY, HEROES[i].anchoCuerpo, C2D_Color32(20, 24, 38, 255));
                dibujarHeroe(hojaHeroes, i, HDIR_ABAJO, sel ? (contadorAnim / 8) % HEROE_CUADROS : 0, pieX, pieY, 1.0f);
                dibujarTextoCentrado(dynamicBuf, pieX, cy + cartaH - 18, 0.42f, clrBlanco, HEROES[i].nombre);
            }
            dibujarTexto(dynamicBuf, 14.0f, 226.0f, 0.38f, clrGris, "Cruceta / toque: elegir     A o tocar de nuevo: comenzar");
            C3D_FrameEnd(0);
            continue;
        }
#if USAR_PERSONALIZACION
        if (estado == ESTADO_PERSONALIZACION) {
            dibujarPanel(8.0f, 4.0f, 304.0f, 26.0f, clrUIBorde, clrUIPanel);
            dibujarTexto(dynamicBuf, 18.0f, 8.0f, 0.5f, clrBlanco, "Personalización");
            for (int i = 0; i < NUM_OPC; i++) {
                float fy = filaY0 + i * filaAlto;
                bool sel = (opcionPJ == i);
                if (i == OPC_COMENZAR) {
                    dibujarPanel(80.0f, fy + 1, 160.0f, filaAlto - 2, sel ? clrBlanco : clrUIBorde, sel ? clrBtnSelect : clrBtnNormal);
                    dibujarTexto(dynamicBuf, 122.0f, fy + 5, 0.5f, clrBlanco, etiquetasPJ[i]);
                    continue;
                }
                dibujarPanel(8.0f, fy + 1, 304.0f, filaAlto - 2, sel ? clrBlanco : clrUIBorde, sel ? clrBtnSelect : clrBtnNormal);
                if (sel) C2D_DrawRectSolid(14.0f, fy + 7, 0.0f, 4.0f, 11.0f, clrBlanco);
                dibujarTexto(dynamicBuf, 24.0f, fy + 5, 0.46f, clrBlanco, etiquetasPJ[i]);

                const char* valor = "";
                const ColorNombrado* muestra = NULL;
                switch (i) {
                    case OPC_NOMBRE:      valor = jugador.nombre; break;
                    case OPC_PIEL:        muestra = &PIELES[apariencia.piel]; break;
                    case OPC_PEINADO:     valor = PEINADOS[apariencia.peinado]; break;
                    case OPC_COLOR_PELO:  muestra = &COLORES_PELO[apariencia.colorPelo]; break;
                    case OPC_ROPA:        valor = ROPAS[apariencia.ropa]; break;
                    case OPC_COLOR_ROPA:  muestra = &COLORES_ROPA[apariencia.colorRopa]; break;
                }
                float vx = 190.0f;
                if (muestra) {
                    C2D_DrawRectSolid(vx, fy + 6, 0.0f, 13.0f, 13.0f, clrBlanco);
                    C2D_DrawRectSolid(vx + 1, fy + 7, 0.0f, 11.0f, 11.0f, colorDe(muestra));
                    valor = muestra->nombre;
                    vx += 18.0f;
                }
                dibujarTexto(dynamicBuf, vx, fy + 5, 0.46f, clrBlanco, valor);
                dibujarTexto(dynamicBuf, 160.0f, fy + 5, 0.46f, sel ? clrBlanco : clrGris, "<");
                dibujarTexto(dynamicBuf, 292.0f, fy + 5, 0.46f, sel ? clrBlanco : clrGris, ">");
            }
            const char* ayuda = (opcionPJ == OPC_NOMBRE) ? "A: escribir nombre   Izq/Der: nombres sugeridos"
                                                          : "Arriba/Abajo: elegir   Izq/Der: cambiar   A: aceptar";
            dibujarTexto(dynamicBuf, 14.0f, 214.0f, 0.4f, clrGris, ayuda);
            C3D_FrameEnd(0);
            continue;
        }
#endif

        // Panel Superior: Estado del Héroe y Barra de HP (con borde)
        C2D_DrawRectSolid(10.0f, 8.0f, 0.0f, 300.0f, 48.0f, clrUIBorde);
        C2D_DrawRectSolid(12.0f, 10.0f, 0.0f, 296.0f, 44.0f, clrUIPanel);

        char statsJugador[64];
        snprintf(statsJugador, sizeof(statsJugador), "%s   HP: %d / %d", jugador.nombre, jugador.hp, jugador.maxHp);
        dibujarTexto(dynamicBuf, 22.0f, 15.0f, 0.50f, clrBlanco, statsJugador);

        // Barra gráfica de HP en el panel inferior
        C2D_DrawRectSolid(22.0f, 36.0f, 0.0f, 276.0f, 10.0f, clrVidaBg);
        C2D_DrawRectSolid(22.0f, 36.0f, 0.0f, 276.0f * ((float)jugador.hp / jugador.maxHp), 10.0f, clrVida);

        // Panel Central: Caja de Registro de Batalla (Log)
        C2D_DrawRectSolid(10.0f, 62.0f, 0.0f, 300.0f, 54.0f, clrUIBorde);
        C2D_DrawRectSolid(12.0f, 64.0f, 0.0f, 296.0f, 50.0f, clrUIPanel);
        dibujarTexto(dynamicBuf, 20.0f, 71.0f, 0.43f, clrBlanco, logLinea1);
        dibujarTexto(dynamicBuf, 20.0f, 91.0f, 0.43f, clrGris, logLinea2);

        // Panel Inferior: Botones de Comandos o Estado de Exploración
        if (estado == ESTADO_COMBATE) {
            for (int i = 0; i < 3; i++) {
                float btnY = 126.0f + (i * 35.0f);
                bool seleccionado = (opcionMenu == i) && fase == F_MENU;

                // Borde brillante y color rojo si está seleccionado
                u32 colorBorde = seleccionado ? clrBlanco : clrUIBorde;
                u32 colorFondo = seleccionado ? clrBtnSelect : clrBtnNormal;

                C2D_DrawRectSolid(20.0f, btnY, 0.0f, 280.0f, 29.0f, colorBorde);
                C2D_DrawRectSolid(22.0f, btnY + 2.0f, 0.0f, 276.0f, 25.0f, colorFondo);

                // Indicador lateral en el botón activo
                if (seleccionado) {
                    C2D_DrawRectSolid(28.0f, btnY + 8.0f, 0.0f, 6.0f, 13.0f, clrBlanco);
                }

                dibujarTexto(dynamicBuf, 44.0f, btnY + 6.0f, 0.48f, clrBlanco, nombresCmd[i]);
            }
        } else {
            // Caja informativa cuando estás caminando en el mapa
            C2D_DrawRectSolid(20.0f, 135.0f, 0.0f, 280.0f, 80.0f, clrUIPanel);
            dibujarTexto(dynamicBuf, 65.0f, 155.0f, 0.52f, clrBlanco, "[ MODO EXPLORACION ]");
            dibujarTexto(dynamicBuf, 45.0f, 178.0f, 0.42f, clrGris, "Presiona START para salir al HBL");
            dibujarTexto(dynamicBuf, 45.0f, 194.0f, 0.42f, clrGris, "SELECT: cambiar de viajero   Y: combate");
        }

        C3D_FrameEnd(0);
    }

    // Liberar recursos al salir
    C2D_TextBufDelete(dynamicBuf);
    C2D_SpriteSheetFree(hojaHeroes);
    for (int i = 0; i < NUM_HEROES; i++) C2D_SpriteSheetFree(hojasCombate[i]);
    C2D_SpriteSheetFree(hojaFx);
#if USAR_PERSONALIZACION
    C2D_SpriteSheetFree(hojaPJ);
#endif
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}