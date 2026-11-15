#ifndef UI_MAPA_H
#define UI_MAPA_H

#include "juego.h"

void renderizarMapaSuperior(const ContextoJuego* ctx, C3D_RenderTarget* top, C2D_TextBuf textBuf);
void renderizarMapaInferior(const ContextoJuego* ctx, C2D_TextBuf textBuf);

#endif