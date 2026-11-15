#include "juego.h"
#include "combate.h"
#include "audio.h"
#include "ui_core.h"
#include "ui_mapa.h"
#include "ui_combate.h"
#include "ui_modales.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

static bool cajasSuperpuestas(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return (ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by);
}

static bool hayColisionMapa(float px, float py) {
    // Hitbox centrada en los pies del héroe (20x16 px)
    float hx = px;
    float hy = py + 4.0f;
    float hw = 20.0f;
    float hh = 16.0f;

    // 1. Límites exteriores del mundo (1200 x 720)
    if (hx < 4.0f || hx + hw > MAPA_ANCHO - 4.0f || hy < 4.0f || hy + hh > MAPA_ALTO - 4.0f) {
        return true;
    }

    // 2. Río 1 (X: 256..282, libre en el Puente 1 Y: 220..256)
    if (cajasSuperpuestas(hx, hy, hw, hh, 256.0f, 0.0f,   26.0f, 218.0f)) return true;
    if (cajasSuperpuestas(hx, hy, hw, hh, 256.0f, 258.0f, 26.0f, 462.0f)) return true;

    // 3. Río 2 (X: 638..664, libre en el Puente 2 Y: 220..256 y en el Puente Sur Y: 520..556)
    if (cajasSuperpuestas(hx, hy, hw, hh, 638.0f, 0.0f,   26.0f, 218.0f)) return true;
    if (cajasSuperpuestas(hx, hy, hw, hh, 638.0f, 258.0f, 26.0f, 260.0f)) return true;
    if (cajasSuperpuestas(hx, hy, hw, hh, 638.0f, 558.0f, 26.0f, 162.0f)) return true;

    // 4. Muro fronterizo de la Mazmorra (X: 916..924, libre en el paso central Y: 220..256)
    if (cajasSuperpuestas(hx, hy, hw, hh, 916.0f, 0.0f,   8.0f, 218.0f)) return true;
    if (cajasSuperpuestas(hx, hy, hw, hh, 916.0f, 258.0f, 8.0f, 462.0f)) return true;

    // 5. Cordillera Norte (X: 290..890, Y: 0..54) y Montañas del Desfiladero Central
    if (cajasSuperpuestas(hx, hy, hw, hh, 290.0f, 0.0f, 595.0f, 54.0f)) return true;
    if (cajasSuperpuestas(hx, hy, hw, hh, 555.0f, 98.0f,  72.0f, 54.0f)) return true;
    if (cajasSuperpuestas(hx, hy, hw, hh, 555.0f, 312.0f, 76.0f, 60.0f)) return true;
    if (cajasSuperpuestas(hx, hy, hw, hh, 675.0f, 322.0f, 80.0f, 58.0f)) return true;
    if (cajasSuperpuestas(hx, hy, hw, hh, 750.0f, 352.0f, 88.0f, 64.0f)) return true;

    // 6. Edificios del Pueblo (Posada, Tienda y Fuente de piedra)
    if (cajasSuperpuestas(hx, hy, hw, hh, 56.0f, 76.0f,  88.0f, 66.0f)) return true; // Posada
    if (cajasSuperpuestas(hx, hy, hw, hh, 56.0f, 316.0f, 88.0f, 66.0f)) return true; // Tienda
    if (cajasSuperpuestas(hx, hy, hw, hh, 30.0f, 222.0f, 28.0f, 28.0f)) return true; // Fuente

    // 7. Fortaleza de la Mazmorra (X: 980, Y: 190; entrada central libre en X: 1005..1045)
    if (cajasSuperpuestas(hx, hy, hw, hh, 974.0f,  182.0f, 102.0f, 43.0f)) return true;
    if (cajasSuperpuestas(hx, hy, hw, hh, 974.0f,  225.0f, 31.0f,  60.0f)) return true;
    if (cajasSuperpuestas(hx, hy, hw, hh, 1045.0f, 225.0f, 31.0f,  60.0f)) return true;

    // 8. Troncos de árboles del Pueblo
    const float puebloArbX[5] = {175.0f, 175.0f, 18.0f, 18.0f, 155.0f};
    const float puebloArbY[5] = {128.0f, 290.0f, 25.0f, 450.0f, 540.0f};
    for (int i = 0; i < 5; i++) {
        if (cajasSuperpuestas(hx, hy, hw, hh, puebloArbX[i] + 18.0f, puebloArbY[i] + 38.0f, 20.0f, 18.0f)) return true;
    }

    // 9. Troncos de árboles de la Pradera y el Valle
    const float praderaArbolX[14] = {
        305.0f, 395.0f, 485.0f, 315.0f, 425.0f, 505.0f, 340.0f, 460.0f,
        695.0f, 790.0f, 845.0f, 705.0f, 825.0f, 760.0f
    };
    const float praderaArbolY[14] = {
        70.0f,  110.0f, 70.0f,  310.0f, 360.0f, 285.0f, 480.0f, 560.0f,
        85.0f,  120.0f, 75.0f,  450.0f, 485.0f, 590.0f
    };
    for (int a = 0; a < 14; a++) {
        if (cajasSuperpuestas(hx, hy, hw, hh, praderaArbolX[a] + 18.0f, praderaArbolY[a] + 38.0f, 20.0f, 18.0f)) return true;
    }

    // 10. Troncos caídos y árboles secos de la Mazmorra
    if (cajasSuperpuestas(hx, hy, hw, hh, 364.0f, 190.0f, 48.0f, 16.0f)) return true;
    if (cajasSuperpuestas(hx, hy, hw, hh, 749.0f, 283.0f, 48.0f, 16.0f)) return true;

    const float secosX[6] = {905.0f, 905.0f, 1090.0f, 955.0f, 1085.0f, 995.0f};
    const float secosY[6] = {110.0f, 305.0f, 145.0f,  45.0f,  365.0f,  490.0f};
    for (int s = 0; s < 6; s++) {
        if (cajasSuperpuestas(hx, hy, hw, hh, secosX[s] + 17.0f, secosY[s] + 38.0f, 20.0f, 18.0f)) return true;
    }

    return false;
}

static void inicializarJuego(ContextoJuego* ctx) {
    memset(ctx, 0, sizeof(ContextoJuego));

    ctx->estado = ESTADO_EXPLORACION;
    ctx->playerX = 140.0f;
    ctx->playerY = 230.0f;
    ctx->cooldownEncuentro = 120;
    ctx->zonaActual = 1;
    ctx->edificioCercano = 0;

    snprintf(ctx->jugador.nombre, sizeof(ctx->jugador.nombre), "Heroe");
    ctx->jugador.nivel       = 1;
    ctx->jugador.exp         = 0;
    ctx->jugador.expSigNivel = 50;
    ctx->jugador.oro         = 50;
    ctx->jugador.pociones    = 2;
    ctx->jugador.eteres      = 1;

    ctx->jugador.maxHp = 100;
    ctx->jugador.hp    = 100;
    ctx->jugador.intel = 10;
    ctx->jugador.maxMp = 40;
    ctx->jugador.mp    = 40;
    ctx->jugador.fue   = 14;
    ctx->jugador.def   = 6;
    ctx->jugador.des   = 10;
    ctx->jugador.mag   = 12;
    ctx->jugador.defM  = 6;
    ctx->jugador.sue   = 7;

    ctx->jugador.equipo.idArma     = 0;
    ctx->jugador.equipo.idArmadura = 0;
    snprintf(ctx->jugador.equipo.accesorio, sizeof(ctx->jugador.equipo.accesorio), "Anillo de Bronce");

    ctx->jugador.equipo.bonoFue  = CATALOGO_ARMAS[0].bonoFue;
    ctx->jugador.equipo.bonoDef  = CATALOGO_ARMADURAS[0].bonoDef;
    ctx->jugador.equipo.bonoDes  = CATALOGO_ARMADURAS[0].bonoDes;
    ctx->jugador.equipo.bonoMag  = CATALOGO_ARMAS[0].bonoMag + 1;
    ctx->jugador.equipo.bonoDefM = 1;
    ctx->jugador.equipo.bonoSue  = 2;

    snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Oeste: Pueblo del Alba | Este: Mazmorra");
    snprintf(ctx->logLinea2, sizeof(ctx->logLinea2), "Pulsa X para ver tus stats y equipo.");
}

static void comprarItemTienda(ContextoJuego* ctx, int opcion) {
    if (opcion == 0) {
        if (ctx->jugador.oro < 25) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Oro insuficiente (necesitas 25 G).");
            return;
        }
        ctx->jugador.oro -= 25;
        ctx->jugador.pociones++;
        audioReproducirSFX(SFX_POCION);
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Compraste 1 Pocion (+45 HP).");
    } else if (opcion == 1) {
        if (ctx->jugador.oro < 35) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Oro insuficiente (necesitas 35 G).");
            return;
        }
        ctx->jugador.oro -= 35;
        ctx->jugador.eteres++;
        audioReproducirSFX(SFX_POCION);
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Compraste 1 Eter (+25 PH).");
    } else if (opcion == 2) {
        int sigArma = ctx->jugador.equipo.idArma + 1;
        if (sigArma >= MAX_ARMAS) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Ya tienes la mejor espada forjada.");
            return;
        }
        const InfoArma* info = &CATALOGO_ARMAS[sigArma];
        if (ctx->jugador.oro < info->precio) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Necesitas %d G para %s.", info->precio, info->nombre);
            return;
        }
        ctx->jugador.oro -= info->precio;
        ctx->jugador.equipo.idArma  = sigArma;
        ctx->jugador.equipo.bonoFue = info->bonoFue;
        ctx->jugador.equipo.bonoMag = info->bonoMag + 1;
        audioReproducirSFX(SFX_HABILIDAD_ESPADA);
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡Equipada %s (+%d FUE)!", info->nombre, info->bonoFue);
    } else if (opcion == 3) {
        int sigArmadura = ctx->jugador.equipo.idArmadura + 1;
        if (sigArmadura >= MAX_ARMADURAS) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Ya tienes la mejor armadura forjada.");
            return;
        }
        const InfoArmadura* info = &CATALOGO_ARMADURAS[sigArmadura];
        if (ctx->jugador.oro < info->precio) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Necesitas %d G para %s.", info->precio, info->nombre);
            return;
        }
        ctx->jugador.oro -= info->precio;
        ctx->jugador.equipo.idArmadura = sigArmadura;
        ctx->jugador.equipo.bonoDef    = info->bonoDef;
        ctx->jugador.equipo.bonoDes    = info->bonoDes;
        audioReproducirSFX(SFX_DEFENDER);
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡Equipada %s (+%d DEF)!", info->nombre, info->bonoDef);
    }
}

static int siguienteEnemigoVivoDir(const ContextoJuego* ctx, int actual, int dir) {
    int idx = actual;
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        idx = (idx + dir + MAX_ENEMIGOS) % MAX_ENEMIGOS;
        if (ctx->enemigos[idx].activo && ctx->enemigos[idx].datos.hp > 0) return idx;
    }
    return actual;
}

int main(void) {
    romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    srand((unsigned int)time(NULL));
    audioInit();
    uiCoreInit();

    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    C2D_TextBuf textBuf = C2D_TextBufNew(4096);

    ContextoJuego ctx;
    inicializarJuego(&ctx);

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if (kDown & KEY_START) break;

        touchPosition touch;
        hidTouchRead(&touch);
        bool tocado = (kDown & KEY_TOUCH) != 0;

        ctx.frameGlobal++;
        actualizarCombate(&ctx);
        audioActualizarEstado(ctx.estado);

        // =========================================================
        // 1. EXPLORACIÓN DEL MAPA (CON COLISIONES Y 4 FASES)
        // =========================================================
        if (ctx.estado == ESTADO_EXPLORACION) {
            float vel = 2.3f;
            float nextX = ctx.playerX;
            float nextY = ctx.playerY;
            float posicionAnteriorX = ctx.playerX;
            float posicionAnteriorY = ctx.playerY;
            ctx.caminando = false;

            if (kHeld & KEY_LEFT)  { nextX -= vel; ctx.caminando = true; ctx.mirandoIzquierda = true; }
            if (kHeld & KEY_RIGHT) { nextX += vel; ctx.caminando = true; ctx.mirandoIzquierda = false; }
            if (kHeld & KEY_UP)    { nextY -= vel; ctx.caminando = true; }
            if (kHeld & KEY_DOWN)  { nextY += vel; ctx.caminando = true; }

            // Deslizamiento independiente por ejes contra ríos, montañas, árboles y edificios
            if (nextX != ctx.playerX && !hayColisionMapa(nextX, ctx.playerY)) {
                ctx.playerX = nextX;
            }
            if (nextY != ctx.playerY && !hayColisionMapa(ctx.playerX, nextY)) {
                ctx.playerY = nextY;
            }
            ctx.distanciaEncuentro += fabsf(ctx.playerX - posicionAnteriorX) +
                                      fabsf(ctx.playerY - posicionAnteriorY);
            if (ctx.cooldownEncuentro > 0) ctx.cooldownEncuentro--;

            // Detección de las 4 fases del mundo (1200 x 720)
            if (ctx.playerX < 258.0f)      ctx.zonaActual = 1; // Pueblo del Alba (Seguro)
            else if (ctx.playerX < 640.0f) ctx.zonaActual = 0; // Pradera Verde (Fase 1)
            else if (ctx.playerX < 920.0f) ctx.zonaActual = 3; // Valle de los Ríos (Fase 2)
            else                           ctx.zonaActual = 2; // Mazmorra Sombría (Fase 3)

            // Proximidad a la Posada o la Tienda
            ctx.edificioCercano = 0;
            if (ctx.playerX >= 68.0f && ctx.playerX <= 125.0f && ctx.playerY >= 136.0f && ctx.playerY <= 168.0f) {
                ctx.edificioCercano = 1;
            } else if (ctx.playerX >= 68.0f && ctx.playerX <= 125.0f && ctx.playerY >= 376.0f && ctx.playerY <= 408.0f) {
                ctx.edificioCercano = 2;
            }

            // Interacción con edificios [A]
            if ((kDown & KEY_A) && ctx.edificioCercano == 1) {
                if (ctx.jugador.oro >= 15) {
                    ctx.jugador.oro -= 15;
                    ctx.jugador.hp = ctx.jugador.maxHp;
                    ctx.jugador.mp = ctx.jugador.maxMp;
                    audioReproducirSFX(SFX_POCION);
                    snprintf(ctx.logLinea1, sizeof(ctx.logLinea1), "¡Descansas en la Posada! HP y PH al 100%%.");
                    snprintf(ctx.logLinea2, sizeof(ctx.logLinea2), "Pagas 15 G al posadero.");
                } else {
                    snprintf(ctx.logLinea1, sizeof(ctx.logLinea1), "Necesitas 15 G para descansar en la Posada.");
                }
            } else if ((kDown & KEY_A) && ctx.edificioCercano == 2) {
                ctx.estado = ESTADO_TIENDA;
                ctx.opcionTienda = 0;
                snprintf(ctx.logLinea1, sizeof(ctx.logLinea1), "Bienvenido a la Herreria y Bazar.");
            }

            // Abrir ficha de personaje [X] o botón táctil
            if ((kDown & KEY_X) || (tocado && touch.px >= 20 && touch.px <= 300 && touch.py >= 184 && touch.py <= 222)) {
                ctx.estado = ESTADO_EQUIPO;
            }

            // Encuentros aleatorios al caminar fuera del Pueblo o con SELECT
            if (kDown & KEY_SELECT) {
                iniciarCombate(&ctx);
            } else if (ctx.caminando && ctx.zonaActual != 1 &&
                       ctx.cooldownEncuentro <= 0 && ctx.distanciaEncuentro >= 160.0f) {
                int probEncuentro = (ctx.zonaActual == 2) ? 30 : ((ctx.zonaActual == 3) ? 24 : 18);
                ctx.distanciaEncuentro = 0.0f;
                if ((rand() % 100) < probEncuentro) {
                    ctx.cooldownEncuentro = 180;
                    iniciarCombate(&ctx);
                }
            }
        }
        // =========================================================
        // 2. MODAL DE TIENDA / HERRERÍA
        // =========================================================
        else if (ctx.estado == ESTADO_TIENDA) {
            if (kDown & KEY_UP)   ctx.opcionTienda = (ctx.opcionTienda + 3) % 4;
            if (kDown & KEY_DOWN) ctx.opcionTienda = (ctx.opcionTienda + 1) % 4;

            if (tocado && touch.px >= 18 && touch.px <= 302) {
                for (int i = 0; i < 4; i++) {
                    float btnY = 38.0f + (i * 33.0f);
                    if (touch.py >= btnY && touch.py <= btnY + 29.0f) {
                        ctx.opcionTienda = i;
                        comprarItemTienda(&ctx, i);
                    }
                }
            } else if (kDown & KEY_A) {
                comprarItemTienda(&ctx, ctx.opcionTienda);
            } else if (kDown & KEY_B) {
                ctx.estado = ESTADO_EXPLORACION;
                snprintf(ctx.logLinea1, sizeof(ctx.logLinea1), "Sales de la Herreria.");
            }
        }
        // =========================================================
        // 3. MODAL DE FICHA DE EQUIPO Y ATRIBUTOS
        // =========================================================
        else if (ctx.estado == ESTADO_EQUIPO) {
            if ((kDown & (KEY_B | KEY_X | KEY_A)) || (tocado && touch.py >= 190)) {
                ctx.estado = ESTADO_EXPLORACION;
            }
        }
        // =========================================================
        // 4. PANTALLA DE VICTORIA (CONTEO PROGRESIVO DE EXP)
        // =========================================================
        else if (ctx.estado == ESTADO_VICTORIA) {
            if ((kDown & KEY_A) || tocado) {
                if (ctx.expPendienteAnim > 0) {
                    // Si pulsa [A] durante el conteo, completa la barra al instante
                    while (ctx.expPendienteAnim > 0) {
                        actualizarCombate(&ctx);
                    }
                } else {
                    if (ctx.infoLevelUp.nivelNuevo > ctx.infoLevelUp.nivelAnterior) {
                        ctx.estado = ESTADO_LEVEL_UP;
                    } else {
                        ctx.estado = ESTADO_EXPLORACION;
                        snprintf(ctx.logLinea1, sizeof(ctx.logLinea1), "Continua tu aventura por el mapa.");
                        snprintf(ctx.logLinea2, sizeof(ctx.logLinea2), "Pulsa X para ver tus stats y equipo.");
                    }
                }
            }
        }
        // =========================================================
        // 5. MODAL DE SUBIDA DE NIVEL
        // =========================================================
        else if (ctx.estado == ESTADO_LEVEL_UP) {
            if ((kDown & (KEY_A | KEY_B)) || tocado) {
                ctx.estado = ESTADO_EXPLORACION;
                snprintf(ctx.logLinea1, sizeof(ctx.logLinea1), "¡Nivel %d alcanzado! Sigues explorando.", ctx.jugador.nivel);
                snprintf(ctx.logLinea2, sizeof(ctx.logLinea2), "Pulsa X para revisar tus nuevos atributos.");
            }
        }
        // =========================================================
        // 6. COMBATE POR TURNOS ATB
        // =========================================================
        else if (ctx.estado == ESTADO_COMBATE) {
            bool turnoJug = (ctx.colaTurnos[0] == TURNO_JUGADOR && !ctx.victoriaPendiente && ctx.timerDerrota <= 0);

            if (turnoJug) {
                if (ctx.seleccionandoObjetivo) {
                    if (kDown & KEY_LEFT)  ctx.objetivoSeleccionado = siguienteEnemigoVivoDir(&ctx, ctx.objetivoSeleccionado, -1);
                    if (kDown & KEY_RIGHT) ctx.objetivoSeleccionado = siguienteEnemigoVivoDir(&ctx, ctx.objetivoSeleccionado, 1);
                    if ((kDown & KEY_A) || tocado) confirmarObjetivoCombate(&ctx);
                    if (kDown & KEY_B) ctx.seleccionandoObjetivo = false;
                }
                else if (ctx.menuHabilidadesAbierto) {
                    int numHab = contarHabilidadesDesbloqueadas(ctx.jugador.nivel);
                    if (kDown & KEY_UP)   ctx.opcionHabilidad = (ctx.opcionHabilidad + numHab - 1) % numHab;
                    if (kDown & KEY_DOWN) ctx.opcionHabilidad = (ctx.opcionHabilidad + 1) % numHab;
                    if (kDown & KEY_Y)    ctx.verDetalleHabilidad = !ctx.verDetalleHabilidad;
                    if (kDown & KEY_A)    seleccionarHabilidadCombate(&ctx, ctx.opcionHabilidad);
                    if (kDown & KEY_B) {
                        if (ctx.verDetalleHabilidad) ctx.verDetalleHabilidad = false;
                        else ctx.menuHabilidadesAbierto = false;
                    }
                }
                else if (ctx.menuObjetosAbierto) {
                    if (kDown & KEY_UP)   ctx.opcionObjeto = (ctx.opcionObjeto + 2) % 3;
                    if (kDown & KEY_DOWN) ctx.opcionObjeto = (ctx.opcionObjeto + 1) % 3;
                    if (kDown & KEY_A)    usarObjetoCombate(&ctx, ctx.opcionObjeto);
                    if (kDown & KEY_B)    ctx.menuObjetosAbierto = false;

                    if (tocado && touch.px >= 20 && touch.px <= 300) {
                        for (int i = 0; i < 3; i++) {
                            float btnY = 132.0f + (i * 32.0f);
                            if (touch.py >= btnY && touch.py <= btnY + 27.0f) {
                                ctx.opcionObjeto = i;
                                usarObjetoCombate(&ctx, i);
                            }
                        }
                    }
                }
                else {
                    if (kDown & KEY_UP)   ctx.opcionMenu = (ctx.opcionMenu + 4) % 5;
                    if (kDown & KEY_DOWN) ctx.opcionMenu = (ctx.opcionMenu + 1) % 5;
                    if (kDown & KEY_A)    ejecutarComandoPrincipal(&ctx, ctx.opcionMenu);

                    if (tocado && touch.px >= 20 && touch.px <= 300) {
                        for (int i = 0; i < 5; i++) {
                            float btnY = 124.0f + (i * 22.0f);
                            if (touch.py >= btnY && touch.py <= btnY + 19.5f) {
                                ctx.opcionMenu = i;
                                ejecutarComandoPrincipal(&ctx, i);
                            }
                        }
                    }
                }
            }
        }

        // =========================================================
        // RENDERIZADO DE AMBAS PANTALLAS
        // =========================================================
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TextBufClear(textBuf);

        // Pantalla Superior (400 x 240)
        C2D_SceneBegin(top);
        if (ctx.estado == ESTADO_EXPLORACION || ctx.estado == ESTADO_TIENDA || ctx.estado == ESTADO_EQUIPO) {
            renderizarMapaSuperior(&ctx, top, textBuf);
        } else {
            renderizarCombateSuperior(&ctx, top, textBuf);
        }

        // Pantalla Inferior (320 x 240)
        C2D_SceneBegin(bottom);
        C2D_TargetClear(bottom, PALETA.uiFondo);

        if (ctx.estado == ESTADO_EXPLORACION) {
            renderizarMapaInferior(&ctx, textBuf);
        } else if (ctx.estado == ESTADO_TIENDA) {
            renderizarModalTienda(&ctx, textBuf);
        } else if (ctx.estado == ESTADO_EQUIPO) {
            renderizarModalEquipo(&ctx, textBuf);
        } else if (ctx.estado == ESTADO_VICTORIA) {
            renderizarModalVictoria(&ctx, textBuf);
        } else if (ctx.estado == ESTADO_LEVEL_UP) {
            renderizarModalLevelUp(&ctx, textBuf);
        } else {
            renderizarCombateInferior(&ctx, textBuf);
        }

        C3D_FrameEnd(0);
    }

    C2D_TextBufDelete(textBuf);
    uiCoreExit();
    audioExit();
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    romfsExit();
    return 0;
}