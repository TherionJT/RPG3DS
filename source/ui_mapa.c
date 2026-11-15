#include "ui_mapa.h"
#include "ui_core.h"
#include <math.h>

static void dibujarTramoRio(float rioX, float yIni, float yFin, float camX, float camY, int frame) {
    float rx = rioX - camX;
    float ry = yIni - camY;
    float alto = yFin - yIni;

    C2D_DrawRectSolid(rx - 4.0f, ry, 0.0f, 34.0f, alto, C2D_Color32(195, 175, 130, 255));
    C2D_DrawRectSolid(rx - 1.0f, ry, 0.0f, 28.0f, alto, C2D_Color32(95, 180, 240, 255));
    C2D_DrawRectSolid(rx + 1.0f, ry, 0.0f, 24.0f, alto, PALETA.agua);

    int despAgua = (frame / 6) % 28;
    for (int wy = (int)yIni; wy < (int)yFin - 10; wy += 28) {
        float yOla = (float)(wy + despAgua) - camY;
        if (yOla >= ry && yOla <= ry + alto - 6.0f) {
            C2D_DrawRectSolid(rx + 4.0f,  yOla,         0.0f, 6.0f, 2.0f, C2D_Color32(150, 220, 255, 190));
            C2D_DrawRectSolid(rx + 14.0f, yOla + 10.0f, 0.0f, 5.0f, 2.0f, C2D_Color32(150, 220, 255, 190));
        }
    }
}

static void dibujarPuenteMadera(float worldX, float worldY, float camX, float camY) {
    float px = worldX - camX;
    float py = worldY - camY;
    float pw = 40.0f;
    float ph = 36.0f;

    C2D_DrawRectSolid(px + 7.0f, py + 4.0f, 0.0f, 26.0f, ph - 2.0f, C2D_Color32(15, 55, 105, 200));
    C2D_DrawRectSolid(px, py + 2.0f, 0.0f, pw, ph - 4.0f, C2D_Color32(75, 45, 20, 255));

    for (int tx = 2; tx < 38; tx += 6) {
        C2D_DrawRectSolid(px + tx, py + 4.0f, 0.0f, 5.0f, ph - 8.0f, C2D_Color32(165, 110, 55, 255));
        C2D_DrawRectSolid(px + tx, py + 6.0f, 0.0f, 5.0f, ph - 12.0f, C2D_Color32(185, 128, 68, 255));
    }

    C2D_DrawRectSolid(px - 1.0f, py,             0.0f, pw + 2.0f, 5.0f, C2D_Color32(110, 68, 30, 255));
    C2D_DrawRectSolid(px - 1.0f, py + ph - 5.0f, 0.0f, pw + 2.0f, 5.0f, C2D_Color32(110, 68, 30, 255));

    u32 grisPilar  = C2D_Color32(130, 135, 145, 255);
    u32 bordePilar = C2D_Color32(70, 75, 85, 255);
    float esquinasX[2] = {px - 2.0f, px + pw - 5.0f};
    float esquinasY[2] = {py - 2.0f, py + ph - 6.0f};
    for (int ix = 0; ix < 2; ix++) {
        for (int iy = 0; iy < 2; iy++) {
            C2D_DrawRectSolid(esquinasX[ix], esquinasY[iy], 0.0f, 7.0f, 8.0f, bordePilar);
            C2D_DrawRectSolid(esquinasX[ix] + 1.0f, esquinasY[iy] + 1.0f, 0.0f, 5.0f, 6.0f, grisPilar);
        }
    }
}

static void dibujarMontana(float wx, float wy, float w, float h, float camX, float camY) {
    float x = wx - camX;
    float y = wy - camY;
    if (x + w < -20.0f || x > 420.0f || y + h < -20.0f || y > 260.0f) return;

    // Base rocosa escalonada con sombra y cumbre nevada
    C2D_DrawRectSolid(x - 2.0f, y + h - 8.0f, 0.0f, w + 4.0f, 10.0f, C2D_Color32(28, 52, 34, 180));
    C2D_DrawRectSolid(x, y + (h * 0.35f), 0.0f, w, h * 0.65f, C2D_Color32(82, 74, 68, 255));
    C2D_DrawRectSolid(x + 4.0f, y + (h * 0.35f), 0.0f, w - 8.0f, h * 0.60f, C2D_Color32(106, 96, 88, 255));

    // Nivel medio de la montaña
    C2D_DrawRectSolid(x + (w * 0.14f), y + (h * 0.15f), 0.0f, w * 0.72f, h * 0.45f, C2D_Color32(124, 114, 104, 255));

    // Cumbre nevada
    C2D_DrawRectSolid(x + (w * 0.26f), y, 0.0f, w * 0.48f, h * 0.26f, C2D_Color32(225, 235, 245, 255));
    C2D_DrawRectSolid(x + (w * 0.32f), y + (h * 0.22f), 0.0f, w * 0.36f, 6.0f, C2D_Color32(190, 205, 222, 255));
}

static void dibujarEdificioDetallado(float x, float y, u32 colorTecho, const char* titulo, C2D_TextBuf textBuf) {
    C2D_DrawRectSolid(x - 3.0f, y + 42.0f, 0.0f, 86.0f, 6.0f, C2D_Color32(0, 0, 0, 75));
    C2D_DrawRectSolid(x, y, 0.0f, 80.0f, 45.0f, C2D_Color32(85, 55, 28, 255));
    C2D_DrawRectSolid(x + 4.0f, y + 2.0f, 0.0f, 72.0f, 36.0f, PALETA.paredCasa);
    C2D_DrawRectSolid(x + 2.0f, y + 37.0f, 0.0f, 76.0f, 8.0f, C2D_Color32(120, 118, 125, 255));

    u32 marcoVentana = C2D_Color32(90, 55, 25, 255);
    u32 luzVentana   = C2D_Color32(255, 225, 115, 255);
    C2D_DrawRectSolid(x + 10.0f, y + 12.0f, 0.0f, 14.0f, 14.0f, marcoVentana);
    C2D_DrawRectSolid(x + 12.0f, y + 14.0f, 0.0f, 10.0f, 10.0f, luzVentana);
    C2D_DrawRectSolid(x + 56.0f, y + 12.0f, 0.0f, 14.0f, 14.0f, marcoVentana);
    C2D_DrawRectSolid(x + 58.0f, y + 14.0f, 0.0f, 10.0f, 10.0f, luzVentana);

    C2D_DrawRectSolid(x + 30.0f, y + 20.0f, 0.0f, 20.0f, 25.0f, marcoVentana);
    C2D_DrawRectSolid(x + 32.0f, y + 22.0f, 0.0f, 16.0f, 23.0f, PALETA.puente);
    C2D_DrawRectSolid(x + 28.0f, y + 44.0f, 0.0f, 24.0f, 4.0f, C2D_Color32(160, 155, 145, 255));

    C2D_DrawRectSolid(x - 6.0f, y - 28.0f, 0.0f, 92.0f, 30.0f, C2D_Color32(30, 32, 42, 255));
    C2D_DrawRectSolid(x - 4.0f, y - 26.0f, 0.0f, 88.0f, 26.0f, colorTecho);
    C2D_DrawRectSolid(x - 2.0f, y - 24.0f, 0.0f, 84.0f, 6.0f, C2D_Color32(255, 255, 255, 40));

    dibujarTexto(textBuf, x + 15.0f, y - 20.0f, 0.38f, PALETA.blanco, titulo);
}

void renderizarMapaSuperior(const ContextoJuego* ctx, C3D_RenderTarget* top, C2D_TextBuf textBuf) {
    C2D_TargetClear(top, PALETA.pasto);

    float camX = floorf(ctx->playerX - 190.0f);
    float camY = floorf(ctx->playerY - 110.0f);
    if (camX < 0.0f) camX = 0.0f;
    if (camY < 0.0f) camY = 0.0f;
    if (camX > MAPA_ANCHO - 400.0f) camX = MAPA_ANCHO - 400.0f;
    if (camY > MAPA_ALTO  - 240.0f) camY = MAPA_ALTO  - 240.0f;

    // 1. TERRENOS BASE DE LAS 4 FASES DEL MAPA (1200 x 720)
    // Fase 1: Pueblo del Alba (X: 0..258)
    C2D_DrawRectSolid(0.0f - camX, 0.0f - camY, 0.0f, 258.0f, MAPA_ALTO, PALETA.puebloSuelo);
    // Fase 3: Valle de los Ríos (tono de pradera más esmeralda X: 640..920)
    C2D_DrawRectSolid(640.0f - camX, 0.0f - camY, 0.0f, 280.0f, MAPA_ALTO, C2D_Color32(36, 124, 52, 255));
    // Fase 4: Mazmorra Sombría (X: 920..1200)
    C2D_DrawRectSolid(920.0f - camX, 0.0f - camY, 0.0f, 280.0f, MAPA_ALTO, C2D_Color32(58, 62, 74, 255));

    // Parches de tierra y hierba
    if (mapaAssetsCargados) {
        const float tierraX[6] = {24.0f, 180.0f, 185.0f, 22.0f, 145.0f, 80.0f};
        const float tierraY[6] = {185.0f, 75.0f, 365.0f, 445.0f, 520.0f, 610.0f};
        for (int i = 0; i < 6; i++) {
            dibujarSpriteMapaProp(1, 0.565f, 0.035f, 0.675f, 0.115f, tierraX[i] - camX, tierraY[i] - camY, 32.0f, 32.0f);
        }

        const float hierbaX[12] = {315, 435, 355, 495, 305, 465, 525, 700, 780, 845, 730, 820};
        const float hierbaY[12] = {150, 175, 275, 135, 445, 520, 265, 160, 290, 420, 540, 190};
        for (int i = 0; i < 12; i++) {
            dibujarSpriteMapaProp(1, 0.068f, 0.025f, 0.220f, 0.122f, hierbaX[i] - camX, hierbaY[i] - camY, 36.0f, 36.0f);
        }
    }

    // 2. CAMINOS EMPEDRADOS CONECTANDO LAS 4 REGIONES
    if (mapaAssetsCargados) {
        // Camino vertical del Pueblo
        for (int py = 144; py <= 320; py += 26) {
            dibujarSpriteMapaProp(2, 0.855f, 0.44f, 0.985f, 0.52f, 86.0f - camX, py - camY, 28.0f, 28.0f);
        }
        // Camino horizontal del Pueblo al Puente 1
        for (int px = 112; px <= 232; px += 24) {
            dibujarSpriteMapaProp(2, 0.15f, 0.910f, 0.42f, 0.972f, px - camX, 226.0f - camY, 26.0f, 26.0f);
        }
        // Plaza central empedrada del Pueblo
        for (int px = 66; px <= 114; px += 24) {
            for (int py = 208; py <= 248; py += 20) {
                dibujarSpriteMapaProp(2, 0.42f, 0.05f, 0.56f, 0.13f, px - camX, py - camY, 26.0f, 24.0f);
            }
        }

        // Sendero Fase 2 (Pradera Verde) -> Puente 2 en X=632
        for (int px = 288; px <= 624; px += 24) {
            dibujarSpriteMapaProp(3, 0.15f, 0.910f, 0.42f, 0.972f, px - camX, 226.0f - camY, 26.0f, 26.0f);
        }
        // Sendero Sur en Fase 3 (Valle de los Ríos: baja desde X=740, Y=240 hasta Y=430 y sigue hacia Mazmorra)
        for (int px = 672; px <= 900; px += 24) {
            dibujarSpriteMapaProp(3, 0.15f, 0.910f, 0.42f, 0.972f, px - camX, 226.0f - camY, 26.0f, 26.0f);
        }

        // Suelo de piedra en Fase 4 (Mazmorra Sombría X: 922..1200)
        int mxIni = (camX > 922.0f) ? ((int)(camX / 32.0f) * 32) : 922;
        for (int mx = mxIni; mx < 1200; mx += 32) {
            for (int my = 0; my < (int)MAPA_ALTO; my += 32) {
                if (mx - camX <= 410.0f && my - camY >= -32.0f && my - camY <= 250.0f) {
                    dibujarSpriteMapaProp(4, 0.11f, 0.055f, 0.21f, 0.105f, mx - camX, my - camY, 32.0f, 32.0f);
                }
            }
        }
        C2D_DrawRectSolid(918.0f - camX, 0.0f - camY, 0.0f, 6.0f, 222.0f, C2D_Color32(45, 42, 55, 255));
        C2D_DrawRectSolid(918.0f - camX, 256.0f - camY, 0.0f, 6.0f, 464.0f, C2D_Color32(45, 42, 55, 255));

        for (int px = 920; px <= 994; px += 26) {
            dibujarSpriteMapaProp(4, 0.15f, 0.802f, 0.42f, 0.848f, px - camX, 228.0f - camY, 28.0f, 24.0f);
        }
    }

    // Fuente animada del Pueblo (X: 30, Y: 222)
    float fntX = 30.0f - camX;
    float fntY = 222.0f - camY;
    C2D_DrawRectSolid(fntX, fntY, 0.0f, 28.0f, 28.0f, C2D_Color32(110, 115, 128, 255));
    C2D_DrawRectSolid(fntX + 3.0f, fntY + 3.0f, 0.0f, 22.0f, 22.0f, PALETA.agua);
    u32 espumaFnt = ((ctx->frameGlobal / 10) % 2 == 0) ? C2D_Color32(170, 235, 255, 255) : C2D_Color32(110, 200, 250, 255);
    C2D_DrawRectSolid(fntX + 10.0f, fntY + 10.0f, 0.0f, 8.0f, 8.0f, C2D_Color32(140, 145, 155, 255));
    C2D_DrawRectSolid(fntX + 12.0f, fntY + 8.0f,  0.0f, 4.0f, 12.0f, espumaFnt);

    // 3. RÍOS, PUENTES Y MUELLE
    // Río 1 (Divisor Pueblo <-> Pradera en X=258, con Puente en Y=220)
    dibujarTramoRio(258.0f, 0.0f, MAPA_ALTO, camX, camY, ctx->frameGlobal);
    dibujarPuenteMadera(250.0f, 220.0f, camX, camY);

    // Muelle de madera en el Río 1 (X: 278, Y: 350)
    float mx = 278.0f - camX, my = 350.0f - camY;
    C2D_DrawRectSolid(mx, my + 3.0f, 0.0f, 24.0f, 14.0f, C2D_Color32(65, 40, 18, 200));
    for (int tx = 0; tx < 22; tx += 5) C2D_DrawRectSolid(mx + tx, my, 0.0f, 4.0f, 14.0f, C2D_Color32(155, 102, 48, 255));

    // Río 2 (Divisor Pradera <-> Valle de los Ríos en X=640, con Puente 2 en Y=220 y Puente Sur en Y=520)
    dibujarTramoRio(640.0f, 0.0f, MAPA_ALTO, camX, camY, ctx->frameGlobal);
    dibujarPuenteMadera(632.0f, 220.0f, camX, camY);
    dibujarPuenteMadera(632.0f, 520.0f, camX, camY);

    // 4. CORDILLERAS MONTAÑOSAS (Norte, Paso Central y Sur)
    // Cordillera Norte a lo largo de la Pradera y el Valle (X: 290..890, Y: 6)
    for (int cx = 295; cx <= 845; cx += 85) {
        if (cx < 590 || cx > 665) {
            dibujarMontana((float)cx, 6.0f, 80.0f, 52.0f, camX, camY);
        }
    }
    // Montañas divisoras entre Fase 2 y Fase 3 (flanqueando el Río 2)
    dibujarMontana(555.0f, 95.0f,  72.0f, 58.0f, camX, camY);
    dibujarMontana(555.0f, 310.0f, 76.0f, 64.0f, camX, camY);
    dibujarMontana(675.0f, 320.0f, 80.0f, 62.0f, camX, camY);
    dibujarMontana(750.0f, 350.0f, 88.0f, 68.0f, camX, camY);

    // 5. EDIFICIOS DEL PUEBLO Y FORTALEZA DE LA MAZMORRA (X: 980, Y: 190)
    dibujarEdificioDetallado(60.0f - camX, 100.0f - camY, PALETA.techoPosada, "POSADA", textBuf);
    dibujarEdificioDetallado(60.0f - camX, 340.0f - camY, PALETA.techoTienda, "TIENDA", textBuf);

    float mazX = 980.0f - camX;
    float mazY = 190.0f - camY;
    C2D_DrawRectSolid(mazX - 6.0f, mazY - 8.0f, 0.0f, 22.0f, 103.0f, C2D_Color32(52, 42, 64, 255));
    C2D_DrawRectSolid(mazX + 74.0f, mazY - 8.0f, 0.0f, 22.0f, 103.0f, C2D_Color32(52, 42, 64, 255));
    C2D_DrawRectSolid(mazX, mazY, 0.0f, 90.0f, 95.0f, PALETA.rocaOscura);
    C2D_DrawRectSolid(mazX + 25.0f, mazY + 35.0f, 0.0f, 40.0f, 60.0f, C2D_Color32(15, 10, 22, 255));

    u32 colorLlama = ((ctx->frameGlobal / 5) % 2 == 0) ? C2D_Color32(255, 190, 40, 255) : C2D_Color32(255, 95, 30, 255);
    C2D_DrawRectSolid(mazX + 16.0f, mazY + 52.0f, 0.0f, 4.0f, 10.0f, PALETA.tronco);
    C2D_DrawRectSolid(mazX + 15.0f, mazY + 46.0f, 0.0f, 6.0f, 6.0f, colorLlama);
    C2D_DrawRectSolid(mazX + 70.0f, mazY + 52.0f, 0.0f, 4.0f, 10.0f, PALETA.tronco);
    C2D_DrawRectSolid(mazX + 69.0f, mazY + 46.0f, 0.0f, 6.0f, 6.0f, colorLlama);
    dibujarTexto(textBuf, mazX + 8.0f, mazY + 8.0f, 0.38f, PALETA.amarillo, "MAZMORRA");

    // 6. ÁRBOLES Y DECORACIÓN NATURAL EN LAS 4 REGIONES
    if (mapaAssetsCargados) {
        // Pueblo del Alba: Manzanos y Cerezos
        dibujarSpriteMapaProp(0, 0.598f, 0.02f, 0.680f, 0.40f, 175.0f - camX, 128.0f - camY, 56.0f, 58.0f);
        dibujarSpriteMapaProp(0, 0.598f, 0.02f, 0.680f, 0.40f, 175.0f - camX, 290.0f - camY, 56.0f, 58.0f);
        dibujarSpriteMapaProp(0, 0.684f, 0.02f, 0.758f, 0.40f, 18.0f - camX,  25.0f - camY,  54.0f, 56.0f);
        dibujarSpriteMapaProp(0, 0.684f, 0.02f, 0.758f, 0.40f, 18.0f - camX,  450.0f - camY, 54.0f, 56.0f);
        dibujarSpriteMapaProp(0, 0.598f, 0.02f, 0.680f, 0.40f, 155.0f - camX, 540.0f - camY, 56.0f, 58.0f);

        // Pradera (Fase 2) y Valle de los Ríos (Fase 3): 14 árboles variados
        const float praderaArbolX[14] = {
            305.0f, 395.0f, 485.0f, 315.0f, 425.0f, 505.0f, 340.0f, 460.0f,
            695.0f, 790.0f, 845.0f, 705.0f, 825.0f, 760.0f
        };
        const float praderaArbolY[14] = {
            70.0f,  110.0f, 70.0f,  310.0f, 360.0f, 285.0f, 480.0f, 560.0f,
            85.0f,  120.0f, 75.0f,  450.0f, 485.0f, 590.0f
        };
        for (int a = 0; a < 14; a++) {
            float uIni = 0.002f, uFin = 0.079f;
            if (a % 4 == 1)      { uIni = 0.084f; uFin = 0.156f; }
            else if (a % 4 == 2) { uIni = 0.762f; uFin = 0.838f; }
            else if (a % 4 == 3) { uIni = 0.345f; uFin = 0.432f; }
            dibujarSpriteMapaProp(0, uIni, 0.00f, uFin, 0.40f,
                                  praderaArbolX[a] - camX, praderaArbolY[a] - camY, 56.0f, 58.0f);
        }

        // Troncos caídos
        dibujarSpriteMapaProp(0, 0.740f, 0.48f, 0.832f, 0.67f, 360.0f - camX, 182.0f - camY, 56.0f, 30.0f);
        dibujarSpriteMapaProp(0, 0.740f, 0.48f, 0.832f, 0.67f, 745.0f - camX, 275.0f - camY, 56.0f, 30.0f);

        // Árboles secos en la Mazmorra Sombría (Fase 4)
        const float secosX[6] = {905.0f, 905.0f, 1090.0f, 955.0f, 1085.0f, 995.0f};
        const float secosY[6] = {110.0f, 305.0f, 145.0f,  45.0f,  365.0f,  490.0f};
        for (int s = 0; s < 6; s++) {
            dibujarSpriteMapaProp(0, 0.440f, 0.00f, 0.514f, 0.40f, secosX[s] - camX, secosY[s] - camY, 54.0f, 58.0f);
        }
    }

    // 7. HÉROE ANIMADO EN EL MAPA
    if (heroCargado) {
        int filaMapa = ctx->caminando ? 1 : 0;
        int maxFrames = ctx->caminando ? 6 : 4;
        int colMapa = (ctx->frameGlobal / 6) % maxFrames;
        dibujarSpriteHeroe(filaMapa, colMapa, ctx->playerX - camX - 14.0f, ctx->playerY - camY - 22.0f, 1.55f, ctx->mirandoIzquierda);
    } else {
        C2D_DrawRectSolid(ctx->playerX - camX, ctx->playerY - camY, 0.0f, 20.0f, 20.0f, PALETA.jugador);
    }

    // Cartel dinámico con las 4 fases del mapa
    C2D_DrawRectSolid(8.0f, 8.0f, 0.0f, 168.0f, 22.0f, PALETA.uiPanel);
    const char* nombreZona = (ctx->zonaActual == 1) ? "PUEBLO DEL ALBA" :
                             (ctx->zonaActual == 2) ? "MAZMORRA SOMBRIA" :
                             (ctx->zonaActual == 3) ? "VALLE DE LOS RIOS" : "PRADERA VERDE";
    u32 colorZona = (ctx->zonaActual == 1) ? PALETA.verdeBonus :
                    (ctx->zonaActual == 2) ? PALETA.btnSelect :
                    (ctx->zonaActual == 3) ? PALETA.amarillo : PALETA.cian;
    dibujarTexto(textBuf, 14.0f, 12.0f, 0.37f, colorZona, nombreZona);

    // Avisos flotantes de interacción
    if (ctx->edificioCercano == 1) {
        C2D_DrawRectSolid(ctx->playerX - camX - 42.0f, ctx->playerY - camY - 34.0f, 0.0f, 108.0f, 18.0f, PALETA.techoPosada);
        dibujarTexto(textBuf, ctx->playerX - camX - 36.0f, ctx->playerY - camY - 31.0f, 0.34f, PALETA.blanco, "[A] Posada (15 Oro)");
    } else if (ctx->edificioCercano == 2) {
        C2D_DrawRectSolid(ctx->playerX - camX - 38.0f, ctx->playerY - camY - 34.0f, 0.0f, 100.0f, 18.0f, PALETA.techoTienda);
        dibujarTexto(textBuf, ctx->playerX - camX - 32.0f, ctx->playerY - camY - 31.0f, 0.34f, PALETA.blanco, "[A] Abrir Tienda");
    }

    // Minimapa proporcional al nuevo mundo (1200 x 720 -> 96 x 58 px)
    C2D_DrawRectSolid(296.0f, 8.0f, 0.0f, 98.0f, 60.0f, PALETA.blanco);
    C2D_DrawRectSolid(297.0f, 9.0f, 0.0f, 21.0f, 58.0f, PALETA.puebloSuelo);
    C2D_DrawRectSolid(318.0f, 9.0f, 0.0f, 30.0f, 58.0f, PALETA.pasto);
    C2D_DrawRectSolid(348.0f, 9.0f, 0.0f, 23.0f, 58.0f, C2D_Color32(36, 124, 52, 255));
    C2D_DrawRectSolid(371.0f, 9.0f, 0.0f, 23.0f, 58.0f, PALETA.mazmorra);
    C2D_DrawRectSolid(297.0f + (ctx->playerX * 0.08f), 9.0f + (ctx->playerY * 0.08f), 0.0f, 3.0f, 3.0f, PALETA.amarillo);
}

void renderizarMapaInferior(const ContextoJuego* ctx, C2D_TextBuf textBuf) {
    dibujarCabeceraHeroe(ctx, textBuf);

    C2D_DrawRectSolid(16.0f, 108.0f, 0.0f, 288.0f, 66.0f, PALETA.uiPanel);
    dibujarTexto(textBuf, 26.0f, 118.0f, 0.38f, PALETA.cian, "Oeste: Pueblo | Centro: Pradera y Valle");
    dibujarTexto(textBuf, 26.0f, 137.0f, 0.38f, PALETA.btnSelect, "Extremo Este: Mazmorra Sombria");
    dibujarTexto(textBuf, 26.0f, 155.0f, 0.37f, PALETA.gris, "SELECT: Combate  |  START: Salir");

    C2D_DrawRectSolid(20.0f, 184.0f, 0.0f, 280.0f, 38.0f, PALETA.amarillo);
    C2D_DrawRectSolid(22.0f, 186.0f, 0.0f, 276.0f, 34.0f, PALETA.btnAccion);
    dibujarTexto(textBuf, 54.0f, 195.0f, 0.45f, PALETA.blanco, "[X] STATS Y EQUIPAMIENTO");
}