#include "ui_combate.h"
#include "ui_core.h"
#include "combate.h"
#include <stdio.h>
#include <string.h>

static void dibujarDecoracionPisoCombate(int zonaActual) {
    if (zonaActual == 2) return;

    for (int x = -8; x < 400; x += 54) {
        dibujarSpriteMapaProp(3, 0.15f, 0.910f, 0.42f, 0.972f,
                              (float)x, 204.0f, 54.0f, 16.0f);
    }

    const float hierbaX[] = {8.0f, 92.0f, 178.0f, 264.0f, 348.0f};
    const float hierbaY[] = {188.0f, 194.0f, 186.0f, 195.0f, 189.0f};
    for (int i = 0; i < 5; i++) {
        dibujarSpriteMapaProp(1, 0.068f, 0.025f, 0.220f, 0.122f,
                              hierbaX[i], hierbaY[i], 48.0f, 22.0f);
    }

    const float tierraX[] = {48.0f, 142.0f, 238.0f, 332.0f};
    const float tierraY[] = {215.0f, 211.0f, 216.0f, 212.0f};
    for (int i = 0; i < 4; i++) {
        dibujarSpriteMapaProp(1, 0.565f, 0.035f, 0.675f, 0.115f,
                              tierraX[i], tierraY[i], 34.0f, 14.0f);
    }
}

void renderizarCombateSuperior(const ContextoJuego* ctx, C3D_RenderTarget* top, C2D_TextBuf textBuf) {
    float arenaShakeY = (ctx->timerHitJugador > 12 && (ctx->timerHitJugador % 2 == 0)) ? 2.0f : 0.0f;

    // Fondo según bioma
    if (ctx->zonaActual != 2) {
        C2D_TargetClear(top, C2D_Color32(43, 152, 224, 255));
        dibujarFondoBatalla(0.0f, arenaShakeY);
    } else {
        C2D_TargetClear(top, PALETA.arenaMaz);
        dibujarFondoMazmorra(0.0f, arenaShakeY);
    }
    dibujarDecoracionPisoCombate(ctx->zonaActual);

    // Posición base del héroe plantado sobre el césped
    float jugX = 62.0f;
    float jugY = 148.0f + arenaShakeY;
    u32 colorRenderJug = PALETA.jugador;

    if (ctx->animAtaqueJugador > 0) {
        int progreso = (ctx->animAtaqueJugador > 12) ? (24 - ctx->animAtaqueJugador) : ctx->animAtaqueJugador;
        jugX += progreso * 3.5f;
    }
    if (ctx->timerHitJugador > 0) {
        jugX += (ctx->timerHitJugador % 4 < 2) ? -4.0f : 4.0f;
        if ((ctx->timerHitJugador / 2) % 2 == 0) colorRenderJug = PALETA.blanco;
    }

    // Héroe animado en batalla (escala grande 3.20x)
    if (heroCargado) {
        int filaBatalla = 0;
        int colBatalla  = (ctx->frameGlobal / 8) % 4;

        if (ctx->jugador.hp <= 0) {
            filaBatalla = 5;
            colBatalla = 2; // Pose caída en el suelo
        } else if (ctx->animAtaqueJugador > 0) {
            filaBatalla = 3; // Ataque con espada (6 frames)
            int frameAtaque = (24 - ctx->animAtaqueJugador) / 4;
            if (frameAtaque < 0) frameAtaque = 0;
            if (frameAtaque > 5) frameAtaque = 5;
            colBatalla = frameAtaque;
        } else if (ctx->timerHitJugador > 0) {
            filaBatalla = 5; // Recibir impacto (3 frames)
            colBatalla = ((18 - ctx->timerHitJugador) / 6) % 3;
        } else if (ctx->jugador.defendiendo) {
            filaBatalla = 4; // Bloqueo con escudo
            colBatalla = 1 + ((ctx->frameGlobal / 10) % 3);
        }

        dibujarSpriteHeroe(filaBatalla, colBatalla, jugX - 31.0f, jugY - 54.0f, 3.20f, false);
    } else {
        C2D_DrawRectSolid(jugX, jugY, 0.0f, 40.0f, 45.0f, colorRenderJug);
    }

    // Escudo de energía al activar postura defensiva
    if (ctx->jugador.defendiendo && ctx->jugador.hp > 0) {
        C2D_DrawRectSolid(jugX + 58.0f, jugY - 12.0f, 0.0f, 6.0f, 58.0f, PALETA.cian);
    }

    // Enemigos alineados sobre el césped
    const float posBaseEneX[3] = {218.0f, 272.0f, 326.0f};
    const float posBaseEneY[3] = {150.0f, 150.0f, 150.0f};
    const float escalaOrco = 1.7f;
    const float anchoCeldaOrco = 100.0f;
    const float anchoZonaEnemigo = 42.0f;
    const float altoZonaEnemigo = 46.0f;

    for (int e = 0; e < MAX_ENEMIGOS; e++) {
        if (!ctx->enemigos[e].activo) continue;
        if (ctx->enemigos[e].datos.hp <= 0 && ctx->enemigos[e].timerMuerte <= 0) continue;

        float ex = posBaseEneX[e];
        float ey = posBaseEneY[e] + arenaShakeY;

        // Animación de muerte: parpadeo rojo/blanco, encogimiento hacia el suelo y desvanecimiento
        if (ctx->enemigos[e].datos.hp <= 0 && ctx->enemigos[e].timerMuerte > 0) {
            float factor = ctx->enemigos[e].timerMuerte / 36.0f;
            if (factor > 1.0f) factor = 1.0f;
            float escalaSprite = escalaOrco * factor;
            float spriteX = ex + (anchoZonaEnemigo - anchoCeldaOrco * escalaSprite) * 0.5f;
            float spriteY = ey + altoZonaEnemigo - 60.0f * escalaSprite;
            int frameMuerte = (int)((1.0f - factor) * 4.0f);
            if (frameMuerte > 3) frameMuerte = 3;
            dibujarSpriteOrco(true, frameMuerte, spriteX, spriteY, escalaSprite, true);
            continue;
        }

        if (ctx->enemigos[e].animAtaque > 0) {
            int prog = (ctx->enemigos[e].animAtaque > 8) ? (16 - ctx->enemigos[e].animAtaque) : ctx->enemigos[e].animAtaque;
            ex -= prog * 4.5f;
        }
        if (ctx->enemigos[e].timerHit > 0) {
            ex += (ctx->enemigos[e].timerHit % 4 < 2) ? -4.0f : 4.0f;
        }
        {
            int colFrame = 0;
            if (ctx->enemigos[e].timerHit > 0) {
                colFrame = 0;
            } else if (ctx->enemigos[e].animAtaque > 0) {
                colFrame = ((16 - ctx->enemigos[e].animAtaque) * 6) / 16;
                if (colFrame > 5) colFrame = 5;
            }

            float spriteX = ex + (anchoZonaEnemigo - anchoCeldaOrco * escalaOrco) * 0.5f;
            float spriteY = ey + altoZonaEnemigo - 60.0f * escalaOrco;
            dibujarSpriteOrco(false, colFrame, spriteX, spriteY, escalaOrco, true);

            float barraX = spriteX + (anchoCeldaOrco * escalaOrco - 50.0f) * 0.5f;
            float barraY = spriteY - 8.0f;
            float ratio = (ctx->enemigos[e].datos.hp > 0) ? ((float)ctx->enemigos[e].datos.hp / ctx->enemigos[e].datos.maxHp) : 0.0f;
            dibujarBarraTexturizada(barraX, barraY, 50.0f, 7.0f, ratio, 1, PALETA.vidaBg, PALETA.vida);
        }

        // Etiqueta E1/E2/E3 con contorno oscuro
        char tagEne[16];
        snprintf(tagEne, sizeof(tagEne), "E%d", e + 1);
        dibujarTexto(textBuf, ex + 14.0f, ey - 21.0f, 0.36f, C2D_Color32(0, 0, 0, 255), tagEne);
        dibujarTexto(textBuf, ex + 13.0f, ey - 22.0f, 0.36f, PALETA.blanco, tagEne);

        if (ctx->seleccionandoObjetivo && ctx->objetivoSeleccionado == e) {
            C2D_DrawRectSolid(ex + 12.0f, ey - 36.0f, 0.0f, 18.0f, 8.0f, PALETA.amarillo);
            dibujarTexto(textBuf, ex + 16.0f, ey - 38.0f, 0.38f, PALETA.uiFondo, "v");
        }
    }

    // HUD superior derecho del enemigo seleccionado
    int infoIdx = ctx->seleccionandoObjetivo ? ctx->objetivoSeleccionado : 0;
    if (!ctx->enemigos[infoIdx].activo || ctx->enemigos[infoIdx].datos.hp <= 0) {
        for (int e = 0; e < MAX_ENEMIGOS; e++) {
            if (ctx->enemigos[e].activo && ctx->enemigos[e].datos.hp > 0) { infoIdx = e; break; }
        }
    }
    if (ctx->enemigos[infoIdx].activo && ctx->enemigos[infoIdx].datos.hp > 0) {
        C2D_DrawRectSolid(220.0f, 12.0f, 0.0f, 170.0f, 38.0f, PALETA.uiBorde);
        C2D_DrawRectSolid(222.0f, 14.0f, 0.0f, 166.0f, 34.0f, PALETA.uiPanel);
        char infoHudEne[40];
        snprintf(infoHudEne, sizeof(infoHudEne), "[E%d] %s", infoIdx + 1, ctx->enemigos[infoIdx].datos.nombre);
        dibujarTexto(textBuf, 228.0f, 17.0f, 0.40f, PALETA.blanco, infoHudEne);

        char hpHudEne[32];
        snprintf(hpHudEne, sizeof(hpHudEne), "HP: %d / %d", ctx->enemigos[infoIdx].datos.hp, ctx->enemigos[infoIdx].datos.maxHp);
        dibujarTexto(textBuf, 228.0f, 32.0f, 0.37f, PALETA.verdeBonus, hpHudEne);
    }

    // Números flotantes de daño / críticos / curación
    for (int i = 0; i < 4; i++) {
        if (ctx->dmgPopups[i].activo) {
            dibujarTexto(textBuf, ctx->dmgPopups[i].x + 1.5f, ctx->dmgPopups[i].y + 1.5f, 0.54f, C2D_Color32(0, 0, 0, 255), ctx->dmgPopups[i].texto);
            dibujarTexto(textBuf, ctx->dmgPopups[i].x, ctx->dmgPopups[i].y, 0.54f, ctx->dmgPopups[i].color, ctx->dmgPopups[i].texto);
        }
    }

    if (ctx->estado == ESTADO_VICTORIA) {
        dibujarTexto(textBuf, 142.0f, 42.0f, 0.65f, PALETA.amarillo, "¡VICTORIA!");
    } else if (ctx->estado == ESTADO_LEVEL_UP) {
        dibujarTexto(textBuf, 145.0f, 45.0f, 0.65f, PALETA.amarillo, "¡LEVEL UP!");
    }
}

void renderizarCombateInferior(const ContextoJuego* ctx, C2D_TextBuf textBuf) {
    dibujarCabeceraHeroe(ctx, textBuf);

    // Barra de cola de turnos
    C2D_DrawRectSolid(8.0f, 99.0f, 0.0f, 304.0f, 22.0f, PALETA.uiPanel);
    dibujarTexto(textBuf, 14.0f, 103.0f, 0.34f, PALETA.amarillo, "TURNOS:");
    C2D_DrawRectSolid(75.0f, 109.0f, 0.0f, 220.0f, 2.0f, PALETA.uiBorde);

    for (int t = 0; t < 6; t++) {
        bool esJugador = (ctx->colaTurnos[t] == TURNO_JUGADOR);
        u32 clrFicha = esJugador ? PALETA.jugador : PALETA.enemigo;
        char etiquetaTurno[4];
        if (esJugador) snprintf(etiquetaTurno, sizeof(etiquetaTurno), "H");
        else snprintf(etiquetaTurno, sizeof(etiquetaTurno), "E%d", (ctx->colaTurnos[t] - TURNO_ENEMIGO_0) + 1);

        if (t == 0) {
            float fx = 70.0f;
            C2D_DrawRectSolid(fx - 2.0f, 100.0f, 0.0f, 34.0f, 20.0f, PALETA.amarillo);
            C2D_DrawRectSolid(fx, 101.0f, 0.0f, 30.0f, 18.0f, clrFicha);
            dibujarTexto(textBuf, fx + (esJugador ? 10.0f : 6.0f), 102.0f, 0.40f, PALETA.blanco, etiquetaTurno);
        } else {
            float fx = 116.0f + ((t - 1) * 36.0f);
            C2D_DrawRectSolid(fx - 1.0f, 101.0f, 0.0f, 26.0f, 18.0f, PALETA.blanco);
            C2D_DrawRectSolid(fx, 102.0f, 0.0f, 24.0f, 16.0f, clrFicha);
            dibujarTexto(textBuf, fx + (esJugador ? 7.0f : 4.0f), 102.0f, 0.36f, PALETA.blanco, etiquetaTurno);
        }
    }

    bool esTurnoJugador = (ctx->colaTurnos[0] == TURNO_JUGADOR && !ctx->victoriaPendiente && ctx->timerDerrota <= 0);

    // Submenú: Selector de objetivo
    if (ctx->seleccionandoObjetivo && esTurnoJugador) {
        C2D_DrawRectSolid(16.0f, 128.0f, 0.0f, 288.0f, 96.0f, PALETA.lvlPanel);
        dibujarTexto(textBuf, 44.0f, 138.0f, 0.45f, PALETA.amarillo, "[ SELECCIONA UN OBJETIVO ]");
        char objName[48];
        snprintf(objName, sizeof(objName), "Objetivo: <  %s  >", ctx->enemigos[ctx->objetivoSeleccionado].datos.nombre);
        dibujarTexto(textBuf, 46.0f, 165.0f, 0.42f, PALETA.blanco, objName);
        dibujarTexto(textBuf, 34.0f, 194.0f, 0.38f, PALETA.gris, "Izq/Der: Cambiar  |  [A]: Atacar  |  [B]: Cancelar");
        return;
    }

    // Submenú: Habilidades y Magia
    if (ctx->menuHabilidadesAbierto && esTurnoJugador) {
        int numHab = contarHabilidadesDesbloqueadas(ctx->jugador.nivel);
        if (ctx->verDetalleHabilidad) {
            const Habilidad* hab = &LISTA_HABILIDADES[ctx->opcionHabilidad];
            C2D_DrawRectSolid(16.0f, 125.0f, 0.0f, 288.0f, 92.0f, PALETA.amarillo);
            C2D_DrawRectSolid(18.0f, 127.0f, 0.0f, 284.0f, 88.0f, PALETA.lvlPanel);

            char titHab[48];
            snprintf(titHab, sizeof(titHab), "%s  -  Coste: %d PH", hab->nombre, hab->costoMp);
            dibujarTexto(textBuf, 26.0f, 133.0f, 0.43f, PALETA.amarillo, titHab);

            char lineaA[52], lineaB[52];
            snprintf(lineaA, sizeof(lineaA), "%.46s", hab->descDetallada);
            dibujarTexto(textBuf, 26.0f, 155.0f, 0.37f, PALETA.blanco, lineaA);
            if (strlen(hab->descDetallada) > 46) {
                snprintf(lineaB, sizeof(lineaB), "%s", hab->descDetallada + 46);
                dibujarTexto(textBuf, 26.0f, 172.0f, 0.37f, PALETA.verdeBonus, lineaB);
            }
            dibujarTexto(textBuf, 44.0f, 195.0f, 0.37f, PALETA.cian, "[A]: Usar Habilidad  |  [Y]: Ocultar detalle");
        } else {
            for (int i = 0; i < numHab; i++) {
                float btnY = 124.0f + (i * 19.0f);
                bool sel = (ctx->opcionHabilidad == i);
                bool alcanzaMp = (ctx->jugador.mp >= LISTA_HABILIDADES[i].costoMp);
                C2D_DrawRectSolid(16.0f, btnY, 0.0f, 288.0f, 17.0f, sel ? PALETA.amarillo : PALETA.uiBorde);
                C2D_DrawRectSolid(18.0f, btnY + 1.0f, 0.0f, 284.0f, 15.0f,
                                  sel ? (LISTA_HABILIDADES[i].esMagia ? PALETA.btnMagia : PALETA.btnAccion) : PALETA.btnNormal);

                char lineaHab[56];
                snprintf(lineaHab, sizeof(lineaHab), "%s (%d PH) - %s",
                         LISTA_HABILIDADES[i].nombre, LISTA_HABILIDADES[i].costoMp, LISTA_HABILIDADES[i].descCorta);
                dibujarTexto(textBuf, 24.0f, btnY + 1.5f, 0.35f, alcanzaMp ? PALETA.blanco : PALETA.gris, lineaHab);
            }
            dibujarTexto(textBuf, 34.0f, 222.0f, 0.36f, PALETA.amarillo, "[A]: Usar  |  [Y]: Ver Potenciador  |  [B]: Volver");
        }
        return;
    }

    // Submenú: Mochila (Objetos)
    if (ctx->menuObjetosAbierto && esTurnoJugador) {
        char opcionesObj[3][48];
        snprintf(opcionesObj[0], sizeof(opcionesObj[0]), "Pocion (+45 HP)   [Disp: %d]", ctx->jugador.pociones);
        snprintf(opcionesObj[1], sizeof(opcionesObj[1]), "Eter   (+25 PH)   [Disp: %d]", ctx->jugador.eteres);
        snprintf(opcionesObj[2], sizeof(opcionesObj[2]), "Volver atras [B]");

        for (int i = 0; i < 3; i++) {
            float btnY = 132.0f + (i * 32.0f);
            bool sel = (ctx->opcionObjeto == i);
            C2D_DrawRectSolid(20.0f, btnY, 0.0f, 280.0f, 27.0f, sel ? PALETA.amarillo : PALETA.uiBorde);
            C2D_DrawRectSolid(22.0f, btnY + 2.0f, 0.0f, 276.0f, 23.0f, sel ? PALETA.btnAccion : PALETA.btnNormal);
            if (sel) C2D_DrawRectSolid(28.0f, btnY + 6.0f, 0.0f, 6.0f, 14.0f, PALETA.amarillo);
            dibujarTexto(textBuf, 42.0f, btnY + 5.0f, 0.42f, PALETA.blanco, opcionesObj[i]);
        }
        return;
    }

    // Menú Principal de Combate
    const char* nombresCmd[5] = {"ATACAR", "HABILIDADES / MAGIA", "DEFENDER (-30% DMG)", "OBJETOS (MOCHILA)", "HUIR"};
    for (int i = 0; i < 5; i++) {
        float btnY = 124.0f + (i * 22.0f);
        bool sel = (ctx->opcionMenu == i && esTurnoJugador);
        C2D_DrawRectSolid(20.0f, btnY, 0.0f, 280.0f, 19.5f, sel ? PALETA.blanco : PALETA.uiBorde);
        C2D_DrawRectSolid(22.0f, btnY + 1.5f, 0.0f, 276.0f, 16.5f, !esTurnoJugador ? PALETA.btnDisabled : (sel ? PALETA.btnSelect : PALETA.btnNormal));
        if (sel) C2D_DrawRectSolid(28.0f, btnY + 4.5f, 0.0f, 6.0f, 10.0f, PALETA.blanco);
        dibujarTexto(textBuf, 42.0f, btnY + 2.0f, 0.39f, esTurnoJugador ? PALETA.blanco : PALETA.gris, nombresCmd[i]);
    }
}