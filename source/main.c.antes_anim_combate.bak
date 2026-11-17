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
    u32 clrEnemigo    = C2D_Color32(220, 50, 50, 255);
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

            if (seMovio) {
                contadorPasos++;
                if (contadorPasos >= 30) {
                    contadorPasos = 0;
                    if ((rand() % 100) < 10) {
                        estado = ESTADO_COMBATE;
                        velX = velY = 0.0f;
                        enemigo.maxHp = 45 + (rand() % 25);
                        enemigo.hp = enemigo.maxHp;
                        opcionMenu = 0;
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
                bool turnoEjecutado = false;

                if (opcionMenu == 0) { // ATACAR
                    int dmg = jugador.atk - enemigo.def + (rand() % 5);
                    if (dmg < 1) dmg = 1;
                    enemigo.hp -= dmg;
                    snprintf(logLinea1, sizeof(logLinea1), "¡Atacas al %s! Causas %d DMG.", enemigo.nombre, dmg);
                    turnoEjecutado = true;
                }
                else if (opcionMenu == 1) { // DEFENDER (-30% daño recibido)
                    jugador.defendiendo = true;
                    snprintf(logLinea1, sizeof(logLinea1), "Adoptas postura defensiva (-30%% daño).");
                    turnoEjecutado = true;
                }
                else if (opcionMenu == 2) { // HUIR
                    if ((rand() % 100) < 65) {
                        estado = ESTADO_EXPLORACION;
                        snprintf(logLinea1, sizeof(logLinea1), "¡Escapaste del combate con exito!");
                        snprintf(logLinea2, sizeof(logLinea2), "Continua explorando el mapa.");
                    } else {
                        snprintf(logLinea1, sizeof(logLinea1), "¡Intentaste huir pero el enemigo te bloqueo!");
                        turnoEjecutado = true;
                    }
                }

                // Comprobar victoria
                if (enemigo.hp <= 0 && estado == ESTADO_COMBATE) {
                    enemigo.hp = 0;
                    estado = ESTADO_EXPLORACION;
                    snprintf(logLinea1, sizeof(logLinea1), "¡Victoria! Derrotaste al %s.", enemigo.nombre);
                    snprintf(logLinea2, sizeof(logLinea2), "Explora el mapa para buscar mas combates.");
                }
                // Contraataque del enemigo
                else if (turnoEjecutado && estado == ESTADO_COMBATE) {
                    int dmgBruto = enemigo.atk - jugador.def + (rand() % 4);
                    if (dmgBruto < 1) dmgBruto = 1;

                    // Aplicar reducción del 30% si está defendiendo (multiplicar por 0.7)
                    int dmgFinal = dmgBruto;
                    if (jugador.defendiendo) {
                        dmgFinal = (int)(dmgBruto * 0.70f);
                        if (dmgFinal < 1) dmgFinal = 1;
                    }

                    jugador.hp -= dmgFinal;

                    if (jugador.hp <= 0) {
                        jugador.hp = jugador.maxHp;
                        estado = ESTADO_EXPLORACION;
                        snprintf(logLinea1, sizeof(logLinea1), "Has sido derrotado...");
                        snprintf(logLinea2, sizeof(logLinea2), "Te recuperas en la zona segura.");
                    } else {
                        if (jugador.defendiendo) {
                            snprintf(logLinea2, sizeof(logLinea2), "%s ataca: ¡Bloqueas! Recibes %d DMG.", enemigo.nombre, dmgFinal);
                        } else {
                            snprintf(logLinea2, sizeof(logLinea2), "¡%s contraataca! Recibes %d DMG.", enemigo.nombre, dmgFinal);
                        }
                    }
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
            // Jugador: viajero elegido a 2x, de perfil mirando al enemigo
            if (heroe >= 0) {
                dibujarSombra(92.0f, 196.0f, HEROES[heroe].anchoCuerpo * 2.0f, C2D_Color32(30, 20, 38, 255));
                dibujarHeroe(hojaHeroes, heroe, HDIR_DER, 0, 92.0f, 196.0f, 2.0f);
                dibujarTextoCentrado(dynamicBuf, 92.0f, 46.0f, 0.5f, clrBlanco, jugador.nombre);
            }
#if USAR_PERSONALIZACION
            else {
                C2D_DrawEllipseSolid(70.0f, 150.0f, 0.0f, 44.0f, 10.0f, C2D_Color32(30, 20, 38, 255));
                dibujarPersonaje(hojaPJ, &apariencia, DIR_E, CUADRO_QUIETO, 60.0f, 96.0f, 2.0f);
            }
#endif
            if (jugador.defendiendo) {
                // Escudo visual celeste cuando se defiende
                C2D_DrawRectSolid(140.0f, 110.0f, 0.0f, 6.0f, 60.0f, C2D_Color32(100, 240, 255, 255));
            }

            // Enemigo + Barra de vida superior
            C2D_DrawRectSolid(275.0f, 95.0f, 0.0f, 55.0f, 55.0f, clrEnemigo);
            C2D_DrawRectSolid(270.0f, 78.0f, 0.0f, 65.0f, 7.0f, clrVidaBg);
            C2D_DrawRectSolid(270.0f, 78.0f, 0.0f, 65.0f * ((float)enemigo.hp / enemigo.maxHp), 7.0f, clrVida);

            char infoEnemigo[32];
            snprintf(infoEnemigo, sizeof(infoEnemigo), "%s (%d/%d)", enemigo.nombre, enemigo.hp, enemigo.maxHp);
            dibujarTexto(dynamicBuf, 255.0f, 58.0f, 0.45f, clrBlanco, infoEnemigo);
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
                bool seleccionado = (opcionMenu == i);

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
            dibujarTexto(dynamicBuf, 45.0f, 194.0f, 0.42f, clrGris, "SELECT: cambiar de viajero");
        }

        C3D_FrameEnd(0);
    }

    // Liberar recursos al salir
    C2D_TextBufDelete(dynamicBuf);
    C2D_SpriteSheetFree(hojaHeroes);
#if USAR_PERSONALIZACION
    C2D_SpriteSheetFree(hojaPJ);
#endif
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}