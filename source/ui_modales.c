#include "ui_modales.h"
#include "ui_core.h"
#include <stdio.h>

void renderizarModalVictoria(const ContextoJuego* ctx, C2D_TextBuf textBuf) {
    C2D_DrawRectSolid(8.0f, 6.0f, 0.0f, 304.0f, 228.0f, PALETA.lvlBorde);
    C2D_DrawRectSolid(10.0f, 8.0f, 0.0f, 300.0f, 224.0f, PALETA.lvlPanel);

    C2D_DrawRectSolid(10.0f, 8.0f, 0.0f, 300.0f, 24.0f, PALETA.lvlFila);
    dibujarTexto(textBuf, 74.0f, 11.0f, 0.46f, PALETA.amarillo, "¡COMBATE GANADO!");

    // Resumen de EXP y Oro obtenidos
    char strExpGanada[48], strOroGanado[48];
    snprintf(strExpGanada, sizeof(strExpGanada), "Experiencia ganada:   +%d EXP  (Restante: %d)",
            ctx->totalExpCombate, ctx->expPendienteAnim);
    snprintf(strOroGanado, sizeof(strOroGanado), "Oro obtenido:         +%d G   (Total: %d G)",
            ctx->oroGanadoFinal, ctx->jugador.oro);

    dibujarTexto(textBuf, 22.0f, 42.0f, 0.39f, PALETA.cian, strExpGanada);
    dibujarTexto(textBuf, 22.0f, 62.0f, 0.39f, PALETA.amarillo, strOroGanado);

    if (ctx->botinVictoria[0] != '\0') {
        dibujarTexto(textBuf, 22.0f, 82.0f, 0.39f, PALETA.verdeBonus, ctx->botinVictoria);
    }

    // Caja de Progreso de Nivel y Barra de EXP animada
    C2D_DrawRectSolid(18.0f, 106.0f, 0.0f, 284.0f, 68.0f, PALETA.lvlFila);
    char strNivelActual[48];
    if (ctx->infoLevelUp.nivelNuevo > ctx->infoLevelUp.nivelAnterior) {
        snprintf(strNivelActual, sizeof(strNivelActual), "%s  -  ¡SUBIO A NIVEL %d!",
                 ctx->jugador.nombre, ctx->jugador.nivel);
        dibujarTexto(textBuf, 26.0f, 114.0f, 0.41f, PALETA.verdeBonus, strNivelActual);
    } else {
        snprintf(strNivelActual, sizeof(strNivelActual), "%s  -  Nivel %d",
                 ctx->jugador.nombre, ctx->jugador.nivel);
        dibujarTexto(textBuf, 26.0f, 114.0f, 0.41f, PALETA.blanco, strNivelActual);
    }

    char strExpBarra[40];
    snprintf(strExpBarra, sizeof(strExpBarra), "EXP:  %d / %d", ctx->jugador.exp, ctx->jugador.expSigNivel);
    dibujarTexto(textBuf, 26.0f, 134.0f, 0.38f, PALETA.gris, strExpBarra);

    float ratioExp = (float)ctx->jugador.exp / (float)ctx->jugador.expSigNivel;
    dibujarBarraTexturizada(26.0f, 152.0f, 268.0f, 12.0f, ratioExp, 3, PALETA.expBg, PALETA.exp);

    // Botón inferior
    if (ctx->expPendienteAnim > 0) {
        dibujarTexto(textBuf, 54.0f, 198.0f, 0.40f, PALETA.gris, "Pulsa [A] para acelerar el conteo...");
    } else {
        C2D_DrawRectSolid(40.0f, 194.0f, 0.0f, 240.0f, 26.0f, PALETA.btnAccion);
        dibujarTexto(textBuf, 64.0f, 199.0f, 0.42f, PALETA.blanco, "Presiona [A] o Toca para seguir");
    }
}

void renderizarModalLevelUp(const ContextoJuego* ctx, C2D_TextBuf textBuf) {
    C2D_DrawRectSolid(8.0f, 4.0f, 0.0f, 304.0f, 232.0f, PALETA.lvlBorde);
    C2D_DrawRectSolid(11.0f, 7.0f, 0.0f, 298.0f, 226.0f, PALETA.lvlPanel);

    C2D_DrawRectSolid(11.0f, 7.0f, 0.0f, 298.0f, 22.0f, PALETA.lvlFila);
    char tituloLvl[48];
    snprintf(tituloLvl, sizeof(tituloLvl), "SUBIDA DE NIVEL:  Nv.%d  ->  Nv.%d",
             ctx->infoLevelUp.nivelAnterior, ctx->infoLevelUp.nivelNuevo);
    dibujarTexto(textBuf, 34.0f, 10.0f, 0.44f, PALETA.amarillo, tituloLvl);

    const char* etiqCol1[5] = {"HP:", "FUE:", "DEF:", "DES:", "SUE:"};
    int valCol1[5] = {ctx->jugador.maxHp, ctx->jugador.fue, ctx->jugador.def, ctx->jugador.des, ctx->jugador.sue};
    int incCol1[5] = {ctx->infoLevelUp.incHp, ctx->infoLevelUp.incFue, ctx->infoLevelUp.incDef, ctx->infoLevelUp.incDes, ctx->infoLevelUp.incSue};

    const char* etiqCol2[4] = {"PH:", "INT:", "MAG:", "DEFM:"};
    int valCol2[4] = {ctx->jugador.maxMp, ctx->jugador.intel, ctx->jugador.mag, ctx->jugador.defM};
    int incCol2[4] = {ctx->infoLevelUp.incMp, ctx->infoLevelUp.incInt, ctx->infoLevelUp.incMag, ctx->infoLevelUp.incDefM};

    for (int r = 0; r < 5; r++) {
        float rowY = 34.0f + (r * 25.0f);
        C2D_DrawRectSolid(16.0f, rowY - 2.0f, 0.0f, 140.0f, 21.0f, PALETA.lvlFila);

        char s1[32], s2[32];
        snprintf(s1, sizeof(s1), "%s %d", etiqCol1[r], valCol1[r]);
        snprintf(s2, sizeof(s2), "(+%d)", incCol1[r]);
        dibujarTexto(textBuf, 22.0f, rowY + 1.0f, 0.39f, PALETA.blanco, s1);
        dibujarTexto(textBuf, 108.0f, rowY + 1.0f, 0.39f, PALETA.verdeBonus, s2);

        if (r < 4) {
            C2D_DrawRectSolid(164.0f, rowY - 2.0f, 0.0f, 140.0f, 21.0f, PALETA.lvlFila);
            char d1[32], d2[32];
            snprintf(d1, sizeof(d1), "%s %d", etiqCol2[r], valCol2[r]);
            snprintf(d2, sizeof(d2), "(+%d)", incCol2[r]);
            dibujarTexto(textBuf, 170.0f, rowY + 1.0f, 0.39f, PALETA.blanco, d1);
            dibujarTexto(textBuf, 256.0f, rowY + 1.0f, 0.39f, PALETA.verdeBonus, d2);
        }
    }

    if (ctx->infoLevelUp.nuevaHabilidad[0] != '\0') {
        C2D_DrawRectSolid(16.0f, 166.0f, 0.0f, 288.0f, 24.0f, PALETA.btnMagia);
        dibujarTexto(textBuf, 36.0f, 171.0f, 0.43f, PALETA.amarillo, ctx->infoLevelUp.nuevaHabilidad);
    }

    C2D_DrawRectSolid(40.0f, 198.0f, 0.0f, 240.0f, 24.0f, PALETA.btnSelect);
    dibujarTexto(textBuf, 68.0f, 202.0f, 0.43f, PALETA.blanco, "Presiona [A] o Toca para seguir");
}

void renderizarModalTienda(const ContextoJuego* ctx, C2D_TextBuf textBuf) {
    C2D_DrawRectSolid(8.0f, 6.0f, 0.0f, 304.0f, 228.0f, PALETA.amarillo);
    C2D_DrawRectSolid(10.0f, 8.0f, 0.0f, 300.0f, 224.0f, PALETA.lvlPanel);

    C2D_DrawRectSolid(10.0f, 8.0f, 0.0f, 300.0f, 24.0f, PALETA.techoTienda);
    char titTienda[48];
    snprintf(titTienda, sizeof(titTienda), "HERRERIA Y BAZAR   |   Oro: %d G", ctx->jugador.oro);
    dibujarTexto(textBuf, 22.0f, 12.0f, 0.43f, PALETA.blanco, titTienda);

    int sigArma = (ctx->jugador.equipo.idArma + 1 < MAX_ARMAS) ? (ctx->jugador.equipo.idArma + 1) : -1;
    int sigArmadura = (ctx->jugador.equipo.idArmadura + 1 < MAX_ARMADURAS) ? (ctx->jugador.equipo.idArmadura + 1) : -1;

    for (int i = 0; i < 4; i++) {
        float btnY = 38.0f + (i * 33.0f);
        bool sel = (ctx->opcionTienda == i);

        C2D_DrawRectSolid(18.0f, btnY, 0.0f, 284.0f, 29.0f, sel ? PALETA.amarillo : PALETA.uiBorde);
        C2D_DrawRectSolid(20.0f, btnY + 1.5f, 0.0f, 280.0f, 26.0f, sel ? PALETA.btnAccion : PALETA.btnNormal);

        char lineaItem[60];
        if (i == 0) {
            snprintf(lineaItem, sizeof(lineaItem), "Pocion (+45 HP) - 25 G  [Tienes: %d]", ctx->jugador.pociones);
            dibujarTexto(textBuf, 28.0f, btnY + 7.0f, 0.36f, PALETA.blanco, lineaItem);
        } else if (i == 1) {
            snprintf(lineaItem, sizeof(lineaItem), "Eter   (+25 PH) - 35 G  [Tienes: %d]", ctx->jugador.eteres);
            dibujarTexto(textBuf, 28.0f, btnY + 7.0f, 0.36f, PALETA.blanco, lineaItem);
        } else if (i == 2) {
            if (sigArma != -1) {
                const InfoArma* arm = &CATALOGO_ARMAS[sigArma];
                dibujarIconoArma(arm->filaIcono, arm->colIcono, 25.0f, btnY + 4.0f, 1.25f);
                snprintf(lineaItem, sizeof(lineaItem), "Forjar %s (+%d FUE) - %d G", arm->nombre, arm->bonoFue, arm->precio);
                dibujarTexto(textBuf, 50.0f, btnY + 7.0f, 0.35f, PALETA.amarillo, lineaItem);
            } else {
                dibujarTexto(textBuf, 28.0f, btnY + 7.0f, 0.36f, PALETA.verdeBonus, "¡Espada maxima alcanzada!");
            }
        } else {
            if (sigArmadura != -1) {
                const InfoArmadura* armd = &CATALOGO_ARMADURAS[sigArmadura];
                dibujarIconoArmadura(armd->filaIcono, armd->colIcono, 25.0f, btnY + 4.0f, 1.25f);
                snprintf(lineaItem, sizeof(lineaItem), "Forjar %s (+%d DEF) - %d G", armd->nombre, armd->bonoDef, armd->precio);
                dibujarTexto(textBuf, 50.0f, btnY + 7.0f, 0.35f, PALETA.cian, lineaItem);
            } else {
                dibujarTexto(textBuf, 28.0f, btnY + 7.0f, 0.36f, PALETA.verdeBonus, "¡Armadura maxima alcanzada!");
            }
        }
    }

    dibujarTexto(textBuf, 24.0f, 174.0f, 0.36f, PALETA.cian, ctx->logLinea1);
    dibujarTexto(textBuf, 54.0f, 204.0f, 0.40f, PALETA.gris, "[A]: Comprar   |   [B]: Salir de la tienda");
}

void renderizarModalEquipo(const ContextoJuego* ctx, C2D_TextBuf textBuf) {
    C2D_DrawRectSolid(8.0f, 6.0f, 0.0f, 304.0f, 228.0f, PALETA.uiBorde);
    C2D_DrawRectSolid(10.0f, 8.0f, 0.0f, 300.0f, 224.0f, PALETA.lvlPanel);

    C2D_DrawRectSolid(10.0f, 8.0f, 0.0f, 300.0f, 22.0f, PALETA.lvlFila);
    char tituloFicha[48];
    snprintf(tituloFicha, sizeof(tituloFicha), "FICHA DE %s (Nv.%d)  -  Oro: %d G",
             ctx->jugador.nombre, ctx->jugador.nivel, ctx->jugador.oro);
    dibujarTexto(textBuf, 18.0f, 11.0f, 0.41f, PALETA.amarillo, tituloFicha);

    dibujarTexto(textBuf, 16.0f, 32.0f, 0.36f, PALETA.cian, "[ ATRIBUTOS ]");

    char sHp[32], sMp[32], sFue[32], sDef[32], sDes[32], sInt[32], sMag[32], sDefM[32], sSue[32];
    snprintf(sHp,   sizeof(sHp),   "HP:   %d/%d", ctx->jugador.hp, ctx->jugador.maxHp);
    snprintf(sMp,   sizeof(sMp),   "PH:   %d/%d", ctx->jugador.mp, ctx->jugador.maxMp);
    snprintf(sFue,  sizeof(sFue),  "FUE:  %d (+%d)", ctx->jugador.fue, ctx->jugador.equipo.bonoFue);
    snprintf(sDef,  sizeof(sDef),  "DEF:  %d (+%d)", ctx->jugador.def, ctx->jugador.equipo.bonoDef);
    snprintf(sDes,  sizeof(sDes),  "DES:  %d (+%d)", ctx->jugador.des, ctx->jugador.equipo.bonoDes);
    snprintf(sInt,  sizeof(sInt),  "INT:  %d", ctx->jugador.intel);
    snprintf(sMag,  sizeof(sMag),  "MAG:  %d (+%d)", ctx->jugador.mag, ctx->jugador.equipo.bonoMag);
    snprintf(sDefM, sizeof(sDefM), "DEFM: %d (+%d)", ctx->jugador.defM, ctx->jugador.equipo.bonoDefM);
    snprintf(sSue,  sizeof(sSue),  "SUE:  %d (+%d)", ctx->jugador.sue, ctx->jugador.equipo.bonoSue);

    dibujarTexto(textBuf, 16.0f, 46.0f,  0.33f, PALETA.blanco, sHp);
    dibujarTexto(textBuf, 16.0f, 58.0f,  0.33f, PALETA.blanco, sMp);
    dibujarTexto(textBuf, 16.0f, 70.0f,  0.33f, PALETA.verdeBonus, sFue);
    dibujarTexto(textBuf, 16.0f, 82.0f,  0.33f, PALETA.verdeBonus, sDef);
    dibujarTexto(textBuf, 16.0f, 94.0f,  0.33f, PALETA.verdeBonus, sDes);
    dibujarTexto(textBuf, 16.0f, 106.0f, 0.33f, PALETA.blanco, sInt);
    dibujarTexto(textBuf, 16.0f, 118.0f, 0.33f, PALETA.verdeBonus, sMag);
    dibujarTexto(textBuf, 16.0f, 130.0f, 0.33f, PALETA.verdeBonus, sDefM);
    dibujarTexto(textBuf, 16.0f, 142.0f, 0.33f, PALETA.amarillo, sSue);

    C2D_DrawRectSolid(135.0f, 32.0f, 0.0f, 2.0f, 124.0f, PALETA.uiBorde);
    dibujarTexto(textBuf, 144.0f, 32.0f, 0.36f, PALETA.cian, "[ EQUIPAMIENTO ]");

    const InfoArma* armaAct = &CATALOGO_ARMAS[ctx->jugador.equipo.idArma];
    dibujarTexto(textBuf, 144.0f, 47.0f, 0.34f, PALETA.gris, "Arma:");
    C2D_DrawRectSolid(144.0f, 60.0f, 0.0f, 22.0f, 22.0f, PALETA.lvlFila);
    dibujarIconoArma(armaAct->filaIcono, armaAct->colIcono, 145.0f, 61.0f, 1.25f);

    char eqArma[44];
    if (armaAct->bonoMag > 0) snprintf(eqArma, sizeof(eqArma), "%s (+%dF/+%dM)", armaAct->nombre, armaAct->bonoFue, armaAct->bonoMag);
    else snprintf(eqArma, sizeof(eqArma), "%s (+%d FUE)", armaAct->nombre, armaAct->bonoFue);
    dibujarTexto(textBuf, 170.0f, 65.0f, 0.33f, PALETA.blanco, eqArma);

    const InfoArmadura* armaduraAct = &CATALOGO_ARMADURAS[ctx->jugador.equipo.idArmadura];
    dibujarTexto(textBuf, 144.0f, 88.0f, 0.34f, PALETA.gris, "Armadura:");
    C2D_DrawRectSolid(144.0f, 100.0f, 0.0f, 22.0f, 22.0f, PALETA.lvlFila);
    dibujarIconoArmadura(armaduraAct->filaIcono, armaduraAct->colIcono, 145.0f, 101.0f, 1.25f);

    char eqArmadura[44];
    snprintf(eqArmadura, sizeof(eqArmadura), "%s (+%dD/+%dV)", armaduraAct->nombre, armaduraAct->bonoDef, armaduraAct->bonoDes);
    dibujarTexto(textBuf, 170.0f, 105.0f, 0.33f, PALETA.blanco, eqArmadura);

    char eqAcc[48];
    snprintf(eqAcc, sizeof(eqAcc), "%s (+%d MAG, +%d SUE)", ctx->jugador.equipo.accesorio, ctx->jugador.equipo.bonoMag, ctx->jugador.equipo.bonoSue);
    dibujarTexto(textBuf, 144.0f, 126.0f, 0.34f, PALETA.gris, "Accesorio:");
    dibujarTexto(textBuf, 144.0f, 138.0f, 0.33f, PALETA.blanco, eqAcc);

    C2D_DrawRectSolid(18.0f, 162.0f, 0.0f, 284.0f, 24.0f, PALETA.lvlFila);
    char mochilaStr[64];
    snprintf(mochilaStr, sizeof(mochilaStr), "MOCHILA:   Pociones (+45 HP): %d   |   Eteres (+25 PH): %d",
            ctx->jugador.pociones, ctx->jugador.eteres);
    dibujarTexto(textBuf, 24.0f, 167.0f, 0.35f, PALETA.amarillo, mochilaStr);

    C2D_DrawRectSolid(40.0f, 196.0f, 0.0f, 240.0f, 26.0f, PALETA.btnSelect);
    dibujarTexto(textBuf, 74.0f, 201.0f, 0.42f, PALETA.blanco, "Presiona [B] o [X] para volver");
}