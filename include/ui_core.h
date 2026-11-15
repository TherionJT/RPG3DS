#ifndef UI_CORE_H
#define UI_CORE_H

#include "juego.h"

typedef struct {
    u32 pasto;
    u32 puebloSuelo;
    u32 mazmorra;
    u32 camino;
    u32 agua;
    u32 puente;
    u32 copaArbol;
    u32 tronco;
    u32 techoPosada;
    u32 techoTienda;
    u32 paredCasa;
    u32 rocaOscura;

    u32 arena;
    u32 arenaMaz;
    u32 suelo;
    u32 jugador;
    u32 enemigo;

    u32 vidaBg;
    u32 vida;
    u32 mpBg;
    u32 mp;
    u32 expBg;
    u32 exp;

    u32 uiFondo;
    u32 uiPanel;
    u32 uiBorde;
    u32 lvlBorde;
    u32 lvlPanel;
    u32 lvlFila;

    u32 btnNormal;
    u32 btnSelect;
    u32 btnAccion;
    u32 btnMagia;
    u32 btnDisabled;

    u32 blanco;
    u32 amarillo;
    u32 verdeBonus;
    u32 cian;
    u32 gris;
} PaletaUI;

extern PaletaUI PALETA;
extern bool weaponsCargado;
extern bool heroCargado;
extern bool mapaAssetsCargados;
extern bool enemiesCargado;

void uiCoreInit(void);
void uiCoreExit(void);

void dibujarSpriteEnemigo(int fila, int col, float x, float y, float escala, bool voltearX);
void dibujarSpriteOrco(bool muerte, int col, float x, float y, float escala, bool voltearX);
void dibujarTexto(C2D_TextBuf buf, float x, float y, float escala, u32 color, const char* str);
void dibujarIconoArma(int fila, int col, float x, float y, float escala);
void dibujarIconoArmadura(int fila, int col, float x, float y, float escala);
void dibujarSpriteHeroe(int fila, int col, float x, float y, float escala, bool voltearX);
void dibujarFondoBatalla(float offsetX, float offsetY);
void dibujarFondoMazmorra(float offsetX, float offsetY);
void dibujarSpriteMapa(int hojaId, int srcX, int srcY, int srcW, int srcH, float dstX, float dstY, float dstW, float dstH);
void dibujarSpriteMapaProp(int hojaId, float u1, float v1, float u2, float v2, float dstX, float dstY, float dstW, float dstH);
void dibujarBarraTexturizada(float x, float y, float ancho, float alto, float porcentaje, int tipoBarra, u32 colorFallbackBg, u32 colorFallbackFill);
void dibujarBarraHeroeCustom(float x, float y, float ancho, float alto, float porcentaje);
void dibujarCabeceraHeroe(const ContextoJuego* ctx, C2D_TextBuf textBuf);

#endif