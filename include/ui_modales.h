#ifndef UI_MODALES_H
#define UI_MODALES_H

#include "juego.h"

void renderizarModalVictoria(const ContextoJuego* ctx, C2D_TextBuf textBuf);
void renderizarModalLevelUp(const ContextoJuego* ctx, C2D_TextBuf textBuf);
void renderizarModalTienda(const ContextoJuego* ctx, C2D_TextBuf textBuf);
void renderizarModalEquipo(const ContextoJuego* ctx, C2D_TextBuf textBuf);

#endif