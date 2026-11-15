#ifndef UI_COMBATE_H
#define UI_COMBATE_H

#include "juego.h"

extern const Habilidad LISTA_HABILIDADES[MAX_HABILIDADES];

int contarHabilidadesDesbloqueadas(int nivel);
void iniciarCombate(ContextoJuego* ctx);
void ejecutarComandoPrincipal(ContextoJuego* ctx, int comando);
void seleccionarHabilidadCombate(ContextoJuego* ctx, int indiceHab);
void confirmarObjetivoCombate(ContextoJuego* ctx);
void usarObjetoCombate(ContextoJuego* ctx, int indiceObjeto);
void actualizarCombate(ContextoJuego* ctx);
void renderizarCombateSuperior(const ContextoJuego* ctx, C3D_RenderTarget* top, C2D_TextBuf textBuf);
void renderizarCombateInferior(const ContextoJuego* ctx, C2D_TextBuf textBuf);

#endif