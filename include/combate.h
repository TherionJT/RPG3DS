#ifndef COMBATE_H
#define COMBATE_H

#include "juego.h"

void iniciarCombate(ContextoJuego* ctx);
void ejecutarComandoPrincipal(ContextoJuego* ctx, int comando);
void seleccionarHabilidadCombate(ContextoJuego* ctx, int indiceHab);
void confirmarObjetivoCombate(ContextoJuego* ctx);
void usarObjetoCombate(ContextoJuego* ctx, int indiceObjeto);
void actualizarCombate(ContextoJuego* ctx);
int contarHabilidadesDesbloqueadas(int nivel);

#endif