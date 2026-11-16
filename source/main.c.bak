#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
    ESTADO_EXPLORACION,
    ESTADO_COMBATE
} EstadoJuego;

typedef struct {
    char nombre[20];
    int hp;
    int maxHp;
    int atk;
    int def;
    bool defendiendo;
} Entidad;

// Función auxiliar para dibujar texto dinámico con la fuente del sistema de 3DS
void dibujarTexto(C2D_TextBuf buf, float x, float y, float escala, u32 color, const char* str) {
    C2D_Text text;
    C2D_TextParse(&text, buf, str);
    C2D_TextOptimize(&text);
    C2D_DrawText(&text, C2D_WithColor, x, y, 0.0f, escala, escala, color);
}

int main(int argc, char* argv[]) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    // Crear lienzos gráficos para AMBAS pantallas
    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    // Buffer para almacenar textos dinámicos de cada frame
    C2D_TextBuf dynamicBuf = C2D_TextBufNew(4096);

    srand(time(NULL));

    EstadoJuego estado = ESTADO_EXPLORACION;
    Entidad jugador = {"Heroe", 100, 100, 18, 5, false};
    Entidad enemigo = {"Goblin", 55, 55, 16, 3, false};

    float playerX = 190.0f, playerY = 110.0f;
    int contadorPasos = 0;
    int opcionMenu = 0; // 0: Atacar, 1: Defender, 2: Huir

    char logLinea1[64] = "Explora el mapa con el D-Pad.";
    char logLinea2[64] = "Los enemigos acechan en la hierba...";

    // Paleta de colores para la UI y el juego
    u32 clrPasto      = C2D_Color32(34, 139, 34, 255);
    u32 clrArena      = C2D_Color32(45, 30, 55, 255);
    u32 clrJugador    = C2D_Color32(50, 150, 255, 255);
    u32 clrEnemigo    = C2D_Color32(220, 50, 50, 255);
    u32 clrVidaBg     = C2D_Color32(60, 20, 20, 255);
    u32 clrVida       = C2D_Color32(0, 220, 90, 255);

    u32 clrUIFondo    = C2D_Color32(18, 20, 32, 255);
    u32 clrUIPanel    = C2D_Color32(32, 38, 58, 255);
    u32 clrUIBorde    = C2D_Color32(90, 110, 160, 255);
    u32 clrBtnNormal  = C2D_Color32(45, 55, 80, 255);
    u32 clrBtnSelect  = C2D_Color32(230, 60, 60, 255);
    u32 clrBlanco     = C2D_Color32(255, 255, 255, 255);
    u32 clrGris       = C2D_Color32(170, 180, 200, 255);

    const char* nombresCmd[3] = {"ATACAR", "DEFENDER (-30% DMG)", "HUIR"};

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if (kDown & KEY_START) break;

        // ==========================================
        // LÓGICA: MODO EXPLORACIÓN
        // ==========================================
        if (estado == ESTADO_EXPLORACION) {
            bool seMovio = false;
            if (kHeld & KEY_RIGHT && playerX < 380) { playerX += 2.0f; seMovio = true; }
            if (kHeld & KEY_LEFT  && playerX > 0)   { playerX -= 2.0f; seMovio = true; }
            if (kHeld & KEY_DOWN  && playerY < 220) { playerY += 2.0f; seMovio = true; }
            if (kHeld & KEY_UP    && playerY > 0)   { playerY -= 2.0f; seMovio = true; }

            if (seMovio) {
                contadorPasos++;
                if (contadorPasos >= 30) {
                    contadorPasos = 0;
                    if ((rand() % 100) < 10) {
                        estado = ESTADO_COMBATE;
                        enemigo.maxHp = 45 + (rand() % 25);
                        enemigo.hp = enemigo.maxHp;
                        opcionMenu = 0;
                        snprintf(logLinea1, sizeof(logLinea1), "¡Un %s salvaje corta el paso!", enemigo.nombre);
                        snprintf(logLinea2, sizeof(logLinea2), "Selecciona un comando de accion.");
                    }
                }
            }
        }
        // ==========================================
        // LÓGICA: MODO COMBATE
        // ==========================================
        else if (estado == ESTADO_COMBATE) {
            if (kDown & KEY_UP)   opcionMenu = (opcionMenu + 2) % 3;
            if (kDown & KEY_DOWN) opcionMenu = (opcionMenu + 1) % 3;

            bool confirmarAccion = (kDown & KEY_A);

            // Soporte para Pantalla Táctil en los 3 botones de comando
            if (kDown & KEY_TOUCH) {
                touchPosition touch;
                hidTouchRead(&touch);
                for (int i = 0; i < 3; i++) {
                    int btnY = 128 + (i * 34);
                    if (touch.px >= 20 && touch.px <= 300 && touch.py >= btnY && touch.py <= btnY + 28) {
                        opcionMenu = i;
                        confirmarAccion = true;
                    }
                }
            }

            if (confirmarAccion) {
                jugador.defendiendo = false;
                bool turnoEjecutado = false;

                if (opcionMenu == 0) { // ATACAR
                    int dmg = jugador.atk - enemigo.def + (rand() % 5);
                    if (dmg < 1) dmg = 1;
                    enemigo.hp -= dmg;
                    snprintf(logLinea1, sizeof(logLinea1), "¡Atacas al %s! Causas %d DMG.", enemigo.nombre, dmg);
                    turnoEjecutado = true;
                }
                else if (opcionMenu == 1) { // DEFENDER (-30% daño recibido)
                    jugador.defendiendo = true;
                    snprintf(logLinea1, sizeof(logLinea1), "Adoptas postura defensiva (-30%% daño).");
                    turnoEjecutado = true;
                }
                else if (opcionMenu == 2) { // HUIR
                    if ((rand() % 100) < 65) {
                        estado = ESTADO_EXPLORACION;
                        snprintf(logLinea1, sizeof(logLinea1), "¡Escapaste del combate con exito!");
                        snprintf(logLinea2, sizeof(logLinea2), "Continua explorando el mapa.");
                    } else {
                        snprintf(logLinea1, sizeof(logLinea1), "¡Intentaste huir pero el enemigo te bloqueo!");
                        turnoEjecutado = true;
                    }
                }

                // Comprobar victoria
                if (enemigo.hp <= 0 && estado == ESTADO_COMBATE) {
                    enemigo.hp = 0;
                    estado = ESTADO_EXPLORACION;
                    snprintf(logLinea1, sizeof(logLinea1), "¡Victoria! Derrotaste al %s.", enemigo.nombre);
                    snprintf(logLinea2, sizeof(logLinea2), "Explora el mapa para buscar mas combates.");
                }
                // Contraataque del enemigo
                else if (turnoEjecutado && estado == ESTADO_COMBATE) {
                    int dmgBruto = enemigo.atk - jugador.def + (rand() % 4);
                    if (dmgBruto < 1) dmgBruto = 1;

                    // Aplicar reducción del 30% si está defendiendo (multiplicar por 0.7)
                    int dmgFinal = dmgBruto;
                    if (jugador.defendiendo) {
                        dmgFinal = (int)(dmgBruto * 0.70f);
                        if (dmgFinal < 1) dmgFinal = 1;
                    }

                    jugador.hp -= dmgFinal;

                    if (jugador.hp <= 0) {
                        jugador.hp = jugador.maxHp;
                        estado = ESTADO_EXPLORACION;
                        snprintf(logLinea1, sizeof(logLinea1), "Has sido derrotado...");
                        snprintf(logLinea2, sizeof(logLinea2), "Te recuperas en la zona segura.");
                    } else {
                        if (jugador.defendiendo) {
                            snprintf(logLinea2, sizeof(logLinea2), "%s ataca: ¡Bloqueas! Recibes %d DMG.", enemigo.nombre, dmgFinal);
                        } else {
                            snprintf(logLinea2, sizeof(logLinea2), "¡%s contraataca! Recibes %d DMG.", enemigo.nombre, dmgFinal);
                        }
                    }
                }
            }
        }

        // ==========================================
        // RENDERIZADO GRÁFICO (AMBAS PANTALLAS)
        // ==========================================
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TextBufClear(dynamicBuf); // Limpiar caché de texto del frame anterior

        // --- 1. PANTALLA SUPERIOR (400x240) ---
        C2D_SceneBegin(top);
        if (estado == ESTADO_EXPLORACION) {
            C2D_TargetClear(top, clrPasto);
            C2D_DrawRectSolid(playerX, playerY, 0.0f, 20.0f, 20.0f, clrJugador);
        } else {
            C2D_TargetClear(top, clrArena);
            // Jugador
            C2D_DrawRectSolid(70.0f, 115.0f, 0.0f, 40.0f, 40.0f, clrJugador);
            if (jugador.defendiendo) {
                // Escudo visual celeste cuando se defiende
                C2D_DrawRectSolid(116.0f, 110.0f, 0.0f, 6.0f, 50.0f, C2D_Color32(100, 240, 255, 255));
            }

            // Enemigo + Barra de vida superior
            C2D_DrawRectSolid(275.0f, 95.0f, 0.0f, 55.0f, 55.0f, clrEnemigo);
            C2D_DrawRectSolid(270.0f, 78.0f, 0.0f, 65.0f, 7.0f, clrVidaBg);
            C2D_DrawRectSolid(270.0f, 78.0f, 0.0f, 65.0f * ((float)enemigo.hp / enemigo.maxHp), 7.0f, clrVida);

            char infoEnemigo[32];
            snprintf(infoEnemigo, sizeof(infoEnemigo), "%s (%d/%d)", enemigo.nombre, enemigo.hp, enemigo.maxHp);
            dibujarTexto(dynamicBuf, 255.0f, 58.0f, 0.45f, clrBlanco, infoEnemigo);
        }

        // --- 2. PANTALLA INFERIOR (UI Estilizada 320x240) ---
        C2D_SceneBegin(bottom);
        C2D_TargetClear(bottom, clrUIFondo);

        // Panel Superior: Estado del Héroe y Barra de HP (con borde)
        C2D_DrawRectSolid(10.0f, 8.0f, 0.0f, 300.0f, 48.0f, clrUIBorde);
        C2D_DrawRectSolid(12.0f, 10.0f, 0.0f, 296.0f, 44.0f, clrUIPanel);

        char statsJugador[64];
        snprintf(statsJugador, sizeof(statsJugador), "%s   HP: %d / %d", jugador.nombre, jugador.hp, jugador.maxHp);
        dibujarTexto(dynamicBuf, 22.0f, 15.0f, 0.50f, clrBlanco, statsJugador);

        // Barra gráfica de HP en el panel inferior
        C2D_DrawRectSolid(22.0f, 36.0f, 0.0f, 276.0f, 10.0f, clrVidaBg);
        C2D_DrawRectSolid(22.0f, 36.0f, 0.0f, 276.0f * ((float)jugador.hp / jugador.maxHp), 10.0f, clrVida);

        // Panel Central: Caja de Registro de Batalla (Log)
        C2D_DrawRectSolid(10.0f, 62.0f, 0.0f, 300.0f, 54.0f, clrUIBorde);
        C2D_DrawRectSolid(12.0f, 64.0f, 0.0f, 296.0f, 50.0f, clrUIPanel);
        dibujarTexto(dynamicBuf, 20.0f, 71.0f, 0.43f, clrBlanco, logLinea1);
        dibujarTexto(dynamicBuf, 20.0f, 91.0f, 0.43f, clrGris, logLinea2);

        // Panel Inferior: Botones de Comandos o Estado de Exploración
        if (estado == ESTADO_COMBATE) {
            for (int i = 0; i < 3; i++) {
                float btnY = 126.0f + (i * 35.0f);
                bool seleccionado = (opcionMenu == i);

                // Borde brillante y color rojo si está seleccionado
                u32 colorBorde = seleccionado ? clrBlanco : clrUIBorde;
                u32 colorFondo = seleccionado ? clrBtnSelect : clrBtnNormal;

                C2D_DrawRectSolid(20.0f, btnY, 0.0f, 280.0f, 29.0f, colorBorde);
                C2D_DrawRectSolid(22.0f, btnY + 2.0f, 0.0f, 276.0f, 25.0f, colorFondo);

                // Indicador lateral en el botón activo
                if (seleccionado) {
                    C2D_DrawRectSolid(28.0f, btnY + 8.0f, 0.0f, 6.0f, 13.0f, clrBlanco);
                }

                dibujarTexto(dynamicBuf, 44.0f, btnY + 6.0f, 0.48f, clrBlanco, nombresCmd[i]);
            }
        } else {
            // Caja informativa cuando estás caminando en el mapa
            C2D_DrawRectSolid(20.0f, 135.0f, 0.0f, 280.0f, 80.0f, clrUIPanel);
            dibujarTexto(dynamicBuf, 65.0f, 155.0f, 0.52f, clrBlanco, "[ MODO EXPLORACION ]");
            dibujarTexto(dynamicBuf, 45.0f, 182.0f, 0.42f, clrGris, "Presiona START para salir al HBL");
        }

        C3D_FrameEnd(0);
    }

    // Liberar recursos al salir
    C2D_TextBufDelete(dynamicBuf);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}