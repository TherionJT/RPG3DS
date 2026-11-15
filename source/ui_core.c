#include "ui_core.h"
#include <stdio.h>
#include <math.h>

const InfoArma CATALOGO_ARMAS[MAX_ARMAS] = {
    {"Espada Madera",    2, 0,   0, 5, 0},
    {"Espada Bronce",    4, 0,  40, 5, 1},
    {"Espada Hierro",    7, 0,  90, 5, 2},
    {"Sable Mitrilo",   10, 3, 160, 5, 3},
    {"Hoja Carmesi",    13, 2, 250, 5, 4},
    {"Espada Dorada",   16, 4, 380, 5, 5},
    {"Filo Esmeralda",  20, 6, 550, 5, 6},
    {"Hoja Ancestral",  25, 9, 800, 5, 7}
};

const InfoArmadura CATALOGO_ARMADURAS[MAX_ARMADURAS] = {
    {"Tunica Viajero",    2, 1,   0,  1, 0},
    {"Jubon de Cuero",    4, 2,  45,  0, 1},
    {"Cota Reforzada",    7, 1,  95,  6, 2},
    {"Coraza de Cobre",  10, 2, 165, 11, 3},
    {"Armadura Acero",   14, 1, 260,  8, 4},
    {"Cota de Mitrilo",  18, 3, 390, 13, 5},
    {"Coraza Dorada",    22, 2, 560, 15, 6},
    {"Armadura Jade",    27, 4, 820, 17, 8}
};

PaletaUI PALETA;
bool weaponsCargado = false;
bool heroCargado = false;
bool mapaAssetsCargados = false;
bool enemiesCargado = false;

static C2D_SpriteSheet sheetWeapons = NULL;
static C2D_Image imgWeaponsCompleta;

static C2D_SpriteSheet sheetArmours = NULL;
static C2D_Image imgArmoursCompleta;
static bool armoursCargado = false;

static C2D_SpriteSheet sheetHero = NULL;
static C2D_Image imgHeroCompleta;

static C2D_SpriteSheet sheetEnemies = NULL;
static C2D_Image imgEnemiesCompleta;
static C2D_SpriteSheet sheetOrcAttack = NULL;
static C2D_Image imgOrcAttack;
static C2D_SpriteSheet sheetOrcDeath = NULL;
static C2D_Image imgOrcDeath;

static C2D_SpriteSheet sheetFondoBatalla = NULL;
static C2D_Image imgFondoBatalla;
static bool fondoBatallaCargado = false;

static C2D_SpriteSheet sheetFondoMazmorra = NULL;
static C2D_Image imgFondoMazmorra;
static bool fondoMazmorraCargado = false;

static C2D_SpriteSheet sheetBars = NULL;
static C2D_Image imgBars;
static bool barsCargado = false;

static C2D_SpriteSheet sheetHpHero = NULL;
static C2D_Image imgHpHero;
static bool hpHeroCargado = false;

static C2D_SpriteSheet sheetsMapa[5] = {NULL, NULL, NULL, NULL, NULL};
static C2D_Image imgsMapa[5];

void uiCoreInit(void) {
    PALETA.pasto       = C2D_Color32(44, 138, 56, 255);
    PALETA.puebloSuelo = C2D_Color32(185, 165, 125, 255);
    PALETA.mazmorra    = C2D_Color32(42, 34, 52, 255);
    PALETA.camino      = C2D_Color32(155, 130, 88, 255);
    PALETA.agua        = C2D_Color32(40, 130, 215, 255);
    PALETA.puente      = C2D_Color32(120, 75, 35, 255);
    PALETA.copaArbol   = C2D_Color32(22, 85, 32, 255);
    PALETA.tronco      = C2D_Color32(95, 60, 28, 255);
    PALETA.techoPosada = C2D_Color32(45, 115, 195, 255);
    PALETA.techoTienda = C2D_Color32(205, 65, 55, 255);
    PALETA.paredCasa   = C2D_Color32(225, 215, 190, 255);
    PALETA.rocaOscura  = C2D_Color32(70, 58, 82, 255);

    PALETA.arena       = C2D_Color32(38, 26, 48, 255);
    PALETA.arenaMaz    = C2D_Color32(24, 16, 30, 255);
    PALETA.suelo       = C2D_Color32(55, 40, 68, 255);
    PALETA.jugador     = C2D_Color32(50, 150, 255, 255);
    PALETA.enemigo     = C2D_Color32(220, 55, 65, 255);

    PALETA.vidaBg      = C2D_Color32(60, 20, 20, 255);
    PALETA.vida        = C2D_Color32(0, 220, 90, 255);
    PALETA.mpBg        = C2D_Color32(25, 25, 65, 255);
    PALETA.mp          = C2D_Color32(150, 90, 255, 255);
    PALETA.expBg       = C2D_Color32(20, 35, 60, 255);
    PALETA.exp         = C2D_Color32(60, 170, 255, 255);

    PALETA.uiFondo     = C2D_Color32(18, 20, 32, 255);
    PALETA.uiPanel     = C2D_Color32(32, 38, 58, 255);
    PALETA.uiBorde     = C2D_Color32(90, 110, 160, 255);
    PALETA.lvlBorde    = C2D_Color32(255, 200, 60, 255);
    PALETA.lvlPanel    = C2D_Color32(26, 32, 52, 255);
    PALETA.lvlFila     = C2D_Color32(38, 46, 72, 255);

    PALETA.btnNormal   = C2D_Color32(45, 55, 80, 255);
    PALETA.btnSelect   = C2D_Color32(230, 60, 60, 255);
    PALETA.btnAccion   = C2D_Color32(45, 110, 190, 255);
    PALETA.btnMagia    = C2D_Color32(110, 55, 175, 255);
    PALETA.btnDisabled = C2D_Color32(28, 32, 46, 255);

    PALETA.blanco      = C2D_Color32(255, 255, 255, 255);
    PALETA.amarillo    = C2D_Color32(255, 215, 70, 255);
    PALETA.verdeBonus  = C2D_Color32(80, 255, 130, 255);
    PALETA.cian        = C2D_Color32(100, 230, 255, 255);
    PALETA.gris        = C2D_Color32(175, 185, 205, 255);

    sheetWeapons = C2D_SpriteSheetLoad("romfs:/weapons.t3x");
    if (sheetWeapons && C2D_SpriteSheetCount(sheetWeapons) >= 1) {
        imgWeaponsCompleta = C2D_SpriteSheetGetImage(sheetWeapons, 0);
        weaponsCargado = true;
    }

    sheetArmours = C2D_SpriteSheetLoad("romfs:/armours.t3x");
    if (sheetArmours && C2D_SpriteSheetCount(sheetArmours) >= 1) {
        imgArmoursCompleta = C2D_SpriteSheetGetImage(sheetArmours, 0);
        armoursCargado = true;
    }

    sheetHero = C2D_SpriteSheetLoad("romfs:/hero.t3x");
    if (sheetHero && C2D_SpriteSheetCount(sheetHero) >= 1) {
        imgHeroCompleta = C2D_SpriteSheetGetImage(sheetHero, 0);
        heroCargado = true;
    }

    sheetEnemies = C2D_SpriteSheetLoad("romfs:/enemies.t3x");
    if (sheetEnemies && C2D_SpriteSheetCount(sheetEnemies) >= 1) {
        imgEnemiesCompleta = C2D_SpriteSheetGetImage(sheetEnemies, 0);
        enemiesCargado = true;
    }

    sheetOrcAttack = C2D_SpriteSheetLoad("romfs:/Orc_Attack01.t3x");
    if (sheetOrcAttack && C2D_SpriteSheetCount(sheetOrcAttack) >= 1) {
        imgOrcAttack = C2D_SpriteSheetGetImage(sheetOrcAttack, 0);
    }

    sheetOrcDeath = C2D_SpriteSheetLoad("romfs:/Orc_Death.t3x");
    if (sheetOrcDeath && C2D_SpriteSheetCount(sheetOrcDeath) >= 1) {
        imgOrcDeath = C2D_SpriteSheetGetImage(sheetOrcDeath, 0);
    }

    sheetFondoBatalla = C2D_SpriteSheetLoad("romfs:/fondo_batalla.t3x");
    if (sheetFondoBatalla && C2D_SpriteSheetCount(sheetFondoBatalla) >= 1) {
        imgFondoBatalla = C2D_SpriteSheetGetImage(sheetFondoBatalla, 0);
        fondoBatallaCargado = true;
    }

    sheetFondoMazmorra = C2D_SpriteSheetLoad("romfs:/fondo_mazmorra.t3x");
    if (sheetFondoMazmorra && C2D_SpriteSheetCount(sheetFondoMazmorra) >= 1) {
        imgFondoMazmorra = C2D_SpriteSheetGetImage(sheetFondoMazmorra, 0);
        fondoMazmorraCargado = true;
    }

    sheetBars = C2D_SpriteSheetLoad("romfs:/bars.t3x");
    if (sheetBars && C2D_SpriteSheetCount(sheetBars) >= 1) {
        imgBars = C2D_SpriteSheetGetImage(sheetBars, 0);
        barsCargado = true;
    }

    sheetHpHero = C2D_SpriteSheetLoad("romfs:/hp_hero.t3x");
    if (sheetHpHero && C2D_SpriteSheetCount(sheetHpHero) >= 1) {
        imgHpHero = C2D_SpriteSheetGetImage(sheetHpHero, 0);
        hpHeroCargado = true;
    }

    const char* rutasMapa[5] = {
        "romfs:/trees.t3x",
        "romfs:/ground_grass.t3x",
        "romfs:/road_pueblo.t3x",
        "romfs:/road_pradera.t3x",
        "romfs:/road_mazmorra.t3x"
    };
    int cargadas = 0;
    for (int i = 0; i < 5; i++) {
        sheetsMapa[i] = C2D_SpriteSheetLoad(rutasMapa[i]);
        if (sheetsMapa[i] && C2D_SpriteSheetCount(sheetsMapa[i]) >= 1) {
            imgsMapa[i] = C2D_SpriteSheetGetImage(sheetsMapa[i], 0);
            cargadas++;
        }
    }
    mapaAssetsCargados = (cargadas > 0);
}

void uiCoreExit(void) {
    if (sheetWeapons)       { C2D_SpriteSheetFree(sheetWeapons);       sheetWeapons = NULL;       weaponsCargado = false; }
    if (sheetArmours)       { C2D_SpriteSheetFree(sheetArmours);       sheetArmours = NULL;       armoursCargado = false; }
    if (sheetHero)          { C2D_SpriteSheetFree(sheetHero);          sheetHero = NULL;          heroCargado = false; }
    if (sheetFondoBatalla)  { C2D_SpriteSheetFree(sheetFondoBatalla);  sheetFondoBatalla = NULL;  fondoBatallaCargado = false; }
    if (sheetFondoMazmorra) { C2D_SpriteSheetFree(sheetFondoMazmorra); sheetFondoMazmorra = NULL; fondoMazmorraCargado = false; }
    if (sheetBars)          { C2D_SpriteSheetFree(sheetBars);          sheetBars = NULL;          barsCargado = false; }
    if (sheetHpHero)        { C2D_SpriteSheetFree(sheetHpHero);        sheetHpHero = NULL;        hpHeroCargado = false; }
    if (sheetEnemies)       { C2D_SpriteSheetFree(sheetEnemies);       sheetEnemies = NULL;       enemiesCargado = false; }
    if (sheetOrcAttack)     { C2D_SpriteSheetFree(sheetOrcAttack);     sheetOrcAttack = NULL; }
    if (sheetOrcDeath)      { C2D_SpriteSheetFree(sheetOrcDeath);      sheetOrcDeath = NULL; }

    for (int i = 0; i < 5; i++) {
        if (sheetsMapa[i]) {
            C2D_SpriteSheetFree(sheetsMapa[i]);
            sheetsMapa[i] = NULL;
        }
    }
    mapaAssetsCargados = false;
}

void dibujarTexto(C2D_TextBuf buf, float x, float y, float escala, u32 color, const char* str) {
    float px = floorf(x);
    float py = floorf(y);

    C2D_Text text;
    C2D_TextParse(&text, buf, str);
    C2D_TextOptimize(&text);

    u32 colorSombra = C2D_Color32(10, 12, 20, 210);
    C2D_DrawText(&text, C2D_WithColor, px + 1.0f, py + 1.0f, 0.0f, escala, escala, colorSombra);
    C2D_DrawText(&text, C2D_WithColor, px, py, 0.0f, escala, escala, color);
}

static void dibujarCeldaAtlas(C2D_Image base, int cols, int filas, int fila, int col, float x, float y, float escala, bool voltearX) {
    int anchoCelda = base.subtex->width / cols;
    int altoCelda  = base.subtex->height / filas;

    Tex3DS_SubTexture subtex;
    subtex.width  = anchoCelda;
    subtex.height = altoCelda;

    float uSpan = base.subtex->right - base.subtex->left;
    float vSpan = base.subtex->bottom - base.subtex->top;

    float u1 = (col * anchoCelda) / (float)base.subtex->width;
    float u2 = ((col + 1) * anchoCelda) / (float)base.subtex->width;
    float v1 = (fila * altoCelda) / (float)base.subtex->height;
    float v2 = ((fila + 1) * altoCelda) / (float)base.subtex->height;

    subtex.left   = base.subtex->left + (voltearX ? u2 : u1) * uSpan;
    subtex.right  = base.subtex->left + (voltearX ? u1 : u2) * uSpan;
    subtex.top    = base.subtex->top  + v1 * vSpan;
    subtex.bottom = base.subtex->top  + v2 * vSpan;

    C2D_Image frame = { base.tex, &subtex };
    C2D_DrawImageAt(frame, x, y, 0.0f, NULL, escala, escala);
}

void dibujarIconoArma(int fila, int col, float x, float y, float escala) {
    if (weaponsCargado) dibujarCeldaAtlas(imgWeaponsCompleta, 8, 9, fila, col, x, y, escala, false);
}

void dibujarIconoArmadura(int fila, int col, float x, float y, float escala) {
    if (armoursCargado) dibujarCeldaAtlas(imgArmoursCompleta, 9, 18, fila, col, x, y, escala, false);
}

void dibujarSpriteHeroe(int fila, int col, float x, float y, float escala, bool voltearX) {
    if (heroCargado) dibujarCeldaAtlas(imgHeroCompleta, 6, 7, fila, col, x, y, escala, voltearX);
}

void dibujarFondoBatalla(float offsetX, float offsetY) {
    if (!fondoBatallaCargado) return;
    float escalaX = 400.0f / (float)imgFondoBatalla.subtex->width;
    float escalaY = 240.0f / (float)imgFondoBatalla.subtex->height;
    C2D_DrawImageAt(imgFondoBatalla, offsetX, offsetY, 0.0f, NULL, escalaX, escalaY);
}

void dibujarFondoMazmorra(float offsetX, float offsetY) {
    if (!fondoMazmorraCargado) return;
    float escalaX = 400.0f / (float)imgFondoMazmorra.subtex->width;
    float escalaY = 240.0f / (float)imgFondoMazmorra.subtex->height;
    C2D_DrawImageAt(imgFondoMazmorra, offsetX, offsetY, 0.0f, NULL, escalaX, escalaY);
}

void dibujarSpriteMapa(int hojaId, int srcX, int srcY, int srcW, int srcH, float dstX, float dstY, float dstW, float dstH) {
    if (hojaId < 0 || hojaId >= 5 || !sheetsMapa[hojaId]) return;

    C2D_Image base = imgsMapa[hojaId];
    Tex3DS_SubTexture subtex;
    subtex.width  = srcW;
    subtex.height = srcH;

    float texW = (float)base.subtex->width;
    float texH = (float)base.subtex->height;
    float uSpan = base.subtex->right - base.subtex->left;
    float vSpan = base.subtex->bottom - base.subtex->top;

    subtex.left   = base.subtex->left + ((srcX + 0.35f) / texW) * uSpan;
    subtex.right  = base.subtex->left + ((srcX + srcW - 0.35f) / texW) * uSpan;
    subtex.top    = base.subtex->top  + ((srcY + 0.35f) / texH) * vSpan;
    subtex.bottom = base.subtex->top  + ((srcY + srcH - 0.35f) / texH) * vSpan;

    C2D_Image tile = { base.tex, &subtex };
    float escX = dstW / (float)srcW;
    float escY = dstH / (float)srcH;
    C2D_DrawImageAt(tile, floorf(dstX), floorf(dstY), 0.0f, NULL, escX, escY);
}

void dibujarSpriteMapaProp(int hojaId, float u1, float v1, float u2, float v2, float dstX, float dstY, float dstW, float dstH) {
    if (hojaId < 0 || hojaId >= 5 || !sheetsMapa[hojaId]) return;

    C2D_Image base = imgsMapa[hojaId];
    const Tex3DS_SubTexture* st = base.subtex;

    float realW = (float)st->width * (u2 - u1);
    float realH = (float)st->height * (v2 - v1);
    if (realW < 1.0f) realW = 1.0f;
    if (realH < 1.0f) realH = 1.0f;

    Tex3DS_SubTexture subtex;
    subtex.width  = (u16)realW;
    subtex.height = (u16)realH;
    subtex.left   = st->left + (st->right  - st->left) * u1;
    subtex.right  = st->left + (st->right  - st->left) * u2;
    subtex.top    = st->top  + (st->bottom - st->top)  * v1;
    subtex.bottom = st->top  + (st->bottom - st->top)  * v2;

    C2D_Image tile = { base.tex, &subtex };
    float escX = dstW / (float)subtex.width;
    float escY = dstH / (float)subtex.height;
    C2D_DrawImageAt(tile, floorf(dstX), floorf(dstY), 0.0f, NULL, escX, escY);
}

void dibujarBarraTexturizada(float x, float y, float ancho, float alto, float porcentaje, int tipoBarra, u32 colorFallbackBg, u32 colorFallbackFill) {
    if (porcentaje < 0.0f) porcentaje = 0.0f;
    if (porcentaje > 1.0f) porcentaje = 1.0f;

    if (!barsCargado) {
        C2D_DrawRectSolid(x, y, 0.0f, ancho, alto, colorFallbackBg);
        if (porcentaje > 0.0f) {
            C2D_DrawRectSolid(x, y, 0.0f, ancho * porcentaje, alto, colorFallbackFill);
        }
        return;
    }

    C2D_DrawRectSolid(x + 1.0f, y + 1.0f, 0.0f, ancho - 2.0f, alto - 2.0f, colorFallbackBg);

    int altoFila = imgBars.subtex->height / 4;
    float uSpan = imgBars.subtex->right - imgBars.subtex->left;
    float vSpan = imgBars.subtex->bottom - imgBars.subtex->top;

    if (porcentaje > 0.0f && tipoBarra >= 1 && tipoBarra <= 3) {
        float v1 = (tipoBarra * altoFila) / (float)imgBars.subtex->height;
        float v2 = ((tipoBarra + 1) * altoFila) / (float)imgBars.subtex->height;

        Tex3DS_SubTexture subFill;
        subFill.width  = (u16)(imgBars.subtex->width * porcentaje);
        subFill.height = altoFila;
        subFill.left   = imgBars.subtex->left;
        subFill.right  = imgBars.subtex->left + (porcentaje * uSpan);
        subFill.top    = imgBars.subtex->top + v1 * vSpan;
        subFill.bottom = imgBars.subtex->top + v2 * vSpan;

        if (subFill.width > 0) {
            C2D_Image imgFill = { imgBars.tex, &subFill };
            float escFillX = (ancho * porcentaje) / (float)subFill.width;
            float escFillY = alto / (float)altoFila;
            C2D_DrawImageAt(imgFill, x, y, 0.0f, NULL, escFillX, escFillY);
        }
    }

    Tex3DS_SubTexture subMarco;
    subMarco.width  = imgBars.subtex->width;
    subMarco.height = altoFila;
    subMarco.left   = imgBars.subtex->left;
    subMarco.right  = imgBars.subtex->right;
    subMarco.top    = imgBars.subtex->top;
    subMarco.bottom = imgBars.subtex->top + ((float)altoFila / imgBars.subtex->height) * vSpan;

    C2D_Image imgMarco = { imgBars.tex, &subMarco };
    float escMarcoX = ancho / (float)imgBars.subtex->width;
    float escMarcoY = alto / (float)altoFila;
    C2D_DrawImageAt(imgMarco, x, y, 0.0f, NULL, escMarcoX, escMarcoY);
}

void dibujarBarraHeroeCustom(float x, float y, float ancho, float alto, float porcentaje) {
    if (porcentaje < 0.0f) porcentaje = 0.0f;
    if (porcentaje > 1.0f) porcentaje = 1.0f;

    if (!hpHeroCargado) {
        C2D_DrawRectSolid(x, y, 0.0f, ancho, alto, PALETA.vidaBg);
        C2D_DrawRectSolid(x, y, 0.0f, ancho * porcentaje, alto, PALETA.vida);
        return;
    }

    float innerX = x + (ancho * 0.285f);
    float innerY = y + (alto  * 0.395f);
    float innerW = ancho * 0.525f;
    float innerH = alto  * 0.115f;

    u32 colorVidaActual = PALETA.vida;
    if (porcentaje <= 0.25f) colorVidaActual = PALETA.btnSelect;
    else if (porcentaje <= 0.50f) colorVidaActual = PALETA.amarillo;

    C2D_DrawRectSolid(innerX, innerY, 0.0f, innerW, innerH, PALETA.vidaBg);
    if (porcentaje > 0.0f) {
        C2D_DrawRectSolid(innerX, innerY, 0.0f, innerW * porcentaje, innerH, colorVidaActual);
    }

    float escalaX = ancho / (float)imgHpHero.subtex->width;
    float escalaY = alto  / (float)imgHpHero.subtex->height;
    C2D_DrawImageAt(imgHpHero, x, y, 0.0f, NULL, escalaX, escalaY);
}

void dibujarCabeceraHeroe(const ContextoJuego* ctx, C2D_TextBuf textBuf) {
    C2D_DrawRectSolid(8.0f, 3.0f, 0.0f, 304.0f, 52.0f, PALETA.uiBorde);
    C2D_DrawRectSolid(10.0f, 5.0f, 0.0f, 300.0f, 48.0f, PALETA.uiPanel);

    char headerHeroe[56];
    snprintf(headerHeroe, sizeof(headerHeroe), "%s - Nv.%d               Oro: %d G",
             ctx->jugador.nombre, ctx->jugador.nivel, ctx->jugador.oro);
    dibujarTexto(textBuf, 16.0f, 7.0f, 0.40f, PALETA.amarillo, headerHeroe);

    dibujarBarraHeroeCustom(-8.0f, 11.0f, 160.0f, 64.0f, (float)ctx->jugador.hp / ctx->jugador.maxHp);

    char strHp[24];
    snprintf(strHp, sizeof(strHp), "HP %d/%d", ctx->jugador.hp, ctx->jugador.maxHp);
    dibujarTexto(textBuf, 54.0f, 20.0f, 0.36f, PALETA.blanco, strHp);

    char strMp[24];
    snprintf(strMp, sizeof(strMp), "PH %d/%d", ctx->jugador.mp, ctx->jugador.maxMp);
    dibujarTexto(textBuf, 166.0f, 20.0f, 0.36f, PALETA.blanco, strMp);
    dibujarBarraTexturizada(222.0f, 22.0f, 82.0f, 6.0f, (float)ctx->jugador.mp / ctx->jugador.maxMp, 2, PALETA.mpBg, PALETA.mp);

    char strExp[32];
    snprintf(strExp, sizeof(strExp), "EXP %d/%d", ctx->jugador.exp, ctx->jugador.expSigNivel);
    dibujarTexto(textBuf, 166.0f, 36.0f, 0.36f, PALETA.gris, strExp);
    dibujarBarraTexturizada(222.0f, 38.0f, 82.0f, 6.0f, (float)ctx->jugador.exp / ctx->jugador.expSigNivel, 3, PALETA.expBg, PALETA.exp);

    C2D_DrawRectSolid(8.0f, 58.0f, 0.0f, 304.0f, 38.0f, PALETA.uiBorde);
    C2D_DrawRectSolid(10.0f, 60.0f, 0.0f, 300.0f, 34.0f, PALETA.uiPanel);
    dibujarTexto(textBuf, 16.0f, 63.0f, 0.40f, PALETA.blanco, ctx->logLinea1);
    dibujarTexto(textBuf, 16.0f, 78.0f, 0.39f, PALETA.gris, ctx->logLinea2);
}

void dibujarSpriteEnemigo(int fila, int col, float x, float y, float escala, bool voltearX) {
    if (enemiesCargado) {
        dibujarCeldaAtlas(imgEnemiesCompleta, 4, 4, fila, col, x, y, escala, voltearX);
    }
}

void dibujarSpriteOrco(bool muerte, int col, float x, float y, float escala, bool voltearX) {
    C2D_Image imagen = muerte ? imgOrcDeath : imgOrcAttack;
    if (imagen.tex) dibujarCeldaAtlas(imagen, muerte ? 4 : 6, 1, 0, col, x, y, escala, voltearX);
}