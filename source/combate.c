#include "combate.h"
#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Habilidad LISTA_HABILIDADES[MAX_HABILIDADES] = {
    {
        "Tajo Rapido", 1, 6, false, OBJ_UN_ENEMIGO,
        "Corte veloz + turno",
        "Potencia: 1.35x FUE (Fisico). Adelanta +35% tu proximo turno."
    },
    {
        "Luz Curativa", 5, 10, true, OBJ_PROPIO,
        "Restaura HP propio",
        "Potencia: 25 + 2.2x MAG (Magia). Restaura salud al instante."
    },
    {
        "Estocada Perforante", 10, 12, false, OBJ_UN_ENEMIGO,
        "Ignora defensa rival",
        "Potencia: 1.60x FUE (Fisico). Penetra por completo la DEF enemiga."
    },
    {
        "Torbellino Acero", 20, 18, false, OBJ_TODOS_ENEMIGOS,
        "Ataque en area",
        "Potencia: 1.25x FUE (Fisico). Golpea a todos los enemigos vivos."
    },
    {
        "Filo Astral", 30, 25, true, OBJ_UN_ENEMIGO,
        "Corte fisico y magico",
        "Potencia: 1.45x (FUE + MAG). Combina poder fisico y magico."
    }
};

typedef struct {
    const char* nombre;
    int baseHp;
    int baseAtk;
    int baseMag;
    int baseDef;
    int baseDes;
    int baseExp;
    int baseOro;
    int probMagia;
    const char* nombreHechizo;
    u32 color;
} PlantillaMonstruo;

static const PlantillaMonstruo listaMonstruos[4] = {
    {"Slime",     36, 11, 13,  3,  5, 24, 12, 25, "Acido Viscoso",   0xFF55DD55},
    {"Goblin",    50, 15, 14,  5,  8, 36, 20, 30, "Chispa Ignea",    0xFF3355EE},
    {"Esqueleto", 68, 19, 22,  8, 10, 54, 32, 45, "Fuego Fatuo",     0xFFDDDDDD},
    {"Caballero", 92, 25, 26, 12, 11, 82, 50, 40, "Onda Sombria",    0xFFAA33AA}
};

static int plantillaEnemigoActual[MAX_ENEMIGOS] = {0, 0, 0};

int contarHabilidadesDesbloqueadas(int nivel) {
    int count = 0;
    for (int i = 0; i < MAX_HABILIDADES; i++) {
        if (nivel >= LISTA_HABILIDADES[i].nivelReq) count++;
    }
    return count;
}

static void lanzarPopup(TextoFlotante* popup, float x, float y, const char* texto, u32 color) {
    popup->activo = true;
    popup->x = x;
    popup->y = y;
    popup->timer = 48;
    popup->color = color;
    snprintf(popup->texto, sizeof(popup->texto), "%s", texto);
}

static int obtenerFueTotal(const Entidad* j)  { return j->fue  + j->equipo.bonoFue; }
static int obtenerDefTotal(const Entidad* j)  { return j->def  + j->equipo.bonoDef; }
static int obtenerDesTotal(const Entidad* j)  { return j->des  + j->equipo.bonoDes; }
static int obtenerMagTotal(const Entidad* j)  { return j->mag  + j->equipo.bonoMag; }
static int obtenerDefMTotal(const Entidad* j) { return j->defM + j->equipo.bonoDefM; }
static int obtenerSueTotal(const Entidad* j)  { return j->sue  + j->equipo.bonoSue; }

static bool evaluarCritico(const Entidad* j) {
    int probCrit = 4 + (int)(obtenerSueTotal(j) * 1.4f);
    if (probCrit > 75) probCrit = 75;
    return ((rand() % 100) < probCrit);
}

static int contarEnemigosVivos(const ContextoJuego* ctx) {
    int vivos = 0;
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (ctx->enemigos[i].activo && ctx->enemigos[i].datos.hp > 0) vivos++;
    }
    return vivos;
}

static int primerEnemigoVivo(const ContextoJuego* ctx) {
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        if (ctx->enemigos[i].activo && ctx->enemigos[i].datos.hp > 0) return i;
    }
    return -1;
}

static void proyectarColaTurnos(ContextoJuego* ctx) {
    float simJug = ctx->medidorJugador;
    float simEne[MAX_ENEMIGOS];
    float velEne[MAX_ENEMIGOS];

    for (int e = 0; e < MAX_ENEMIGOS; e++) {
        simEne[e] = ctx->enemigos[e].medidorTurno;
        velEne[e] = 10.0f + (ctx->enemigos[e].datos.des * 1.5f);
    }
    float velJug = 10.0f + (obtenerDesTotal(&ctx->jugador) * 1.5f);

    for (int slot = 0; slot < 6; slot++) {
        while (true) {
            bool alguienListo = (simJug >= 100.0f);
            for (int e = 0; e < MAX_ENEMIGOS; e++) {
                if (ctx->enemigos[e].activo && ctx->enemigos[e].datos.hp > 0 && simEne[e] >= 100.0f) {
                    alguienListo = true;
                }
            }
            if (alguienListo) break;

            simJug += velJug;
            for (int e = 0; e < MAX_ENEMIGOS; e++) {
                if (ctx->enemigos[e].activo && ctx->enemigos[e].datos.hp > 0) {
                    simEne[e] += velEne[e];
                }
            }
        }

        float maxMedidor = simJug;
        TipoTurno ganador = TURNO_JUGADOR;

        for (int e = 0; e < MAX_ENEMIGOS; e++) {
            if (ctx->enemigos[e].activo && ctx->enemigos[e].datos.hp > 0 && simEne[e] > maxMedidor) {
                maxMedidor = simEne[e];
                ganador = (TipoTurno)(TURNO_ENEMIGO_0 + e);
            }
        }

        ctx->colaTurnos[slot] = ganador;
        if (ganador == TURNO_JUGADOR) {
            simJug -= 100.0f;
        } else {
            simEne[ganador - TURNO_ENEMIGO_0] -= 100.0f;
        }
    }
}

static void avanzarColaTurnosTrasAccion(ContextoJuego* ctx) {
    float velJug = 10.0f + (obtenerDesTotal(&ctx->jugador) * 1.5f);
    float velEne[MAX_ENEMIGOS];
    for (int e = 0; e < MAX_ENEMIGOS; e++) {
        velEne[e] = 10.0f + (ctx->enemigos[e].datos.des * 1.5f);
    }

    while (true) {
        bool alguienListo = (ctx->medidorJugador >= 100.0f);
        for (int e = 0; e < MAX_ENEMIGOS; e++) {
            if (ctx->enemigos[e].activo && ctx->enemigos[e].datos.hp > 0 && ctx->enemigos[e].medidorTurno >= 100.0f) {
                alguienListo = true;
            }
        }
        if (alguienListo) break;

        ctx->medidorJugador += velJug;
        for (int e = 0; e < MAX_ENEMIGOS; e++) {
            if (ctx->enemigos[e].activo && ctx->enemigos[e].datos.hp > 0) {
                ctx->enemigos[e].medidorTurno += velEne[e];
            }
        }
    }

    float maxMedidor = ctx->medidorJugador;
    TipoTurno ganador = TURNO_JUGADOR;
    for (int e = 0; e < MAX_ENEMIGOS; e++) {
        if (ctx->enemigos[e].activo && ctx->enemigos[e].datos.hp > 0 && ctx->enemigos[e].medidorTurno > maxMedidor) {
            maxMedidor = ctx->enemigos[e].medidorTurno;
            ganador = (TipoTurno)(TURNO_ENEMIGO_0 + e);
        }
    }

    if (ganador == TURNO_JUGADOR) {
        ctx->medidorJugador -= 100.0f;
    } else {
        ctx->enemigos[ganador - TURNO_ENEMIGO_0].medidorTurno -= 100.0f;
    }

    proyectarColaTurnos(ctx);

    if (ctx->colaTurnos[0] == TURNO_JUGADOR) {
        ctx->jugador.defendiendo = false;
        snprintf(ctx->logLinea2, sizeof(ctx->logLinea2), "Tu turno. Elige una accion.");
    } else {
        int idxEne = ctx->colaTurnos[0] - TURNO_ENEMIGO_0;
        ctx->timerTurnoEnemigo = 42;
        snprintf(ctx->logLinea2, sizeof(ctx->logLinea2), "Turno de %s...", ctx->enemigos[idxEne].datos.nombre);
    }
}

void iniciarCombate(ContextoJuego* ctx) {
    bool enMazmorra = (ctx->zonaActual == 2);

    int tiradaGrupo = rand() % 100;
    int cantidadEnemigos = 1;
    if (enMazmorra) {
        if (tiradaGrupo >= 70) cantidadEnemigos = 3;
        else if (tiradaGrupo >= 35) cantidadEnemigos = 2;
    } else {
        if (tiradaGrupo >= 88) cantidadEnemigos = 3;
        else if (tiradaGrupo >= 60) cantidadEnemigos = 2;
    }

    ctx->totalExpCombate = 0;
    ctx->totalOroCombate = 0;

    for (int e = 0; e < MAX_ENEMIGOS; e++) {
        ctx->dmgPopups[e].activo = false;
        if (e < cantidadEnemigos) {
            ctx->enemigos[e].activo = true;

            int idx = enMazmorra ? (2 + (rand() % 2)) : (rand() % 2);
            plantillaEnemigoActual[e] = idx;

            int lvl = ctx->jugador.nivel + (enMazmorra ? 1 : 0) + (rand() % 2);
            if (lvl < 1) lvl = 1;

            ctx->enemigos[e].datos.nivel = lvl;
            if (cantidadEnemigos > 1) {
                snprintf(ctx->enemigos[e].datos.nombre, sizeof(ctx->enemigos[e].datos.nombre),
                         "%s %c Nv.%d", listaMonstruos[idx].nombre, 'A' + e, lvl);
            } else {
                snprintf(ctx->enemigos[e].datos.nombre, sizeof(ctx->enemigos[e].datos.nombre),
                         "%s Nv.%d", listaMonstruos[idx].nombre, lvl);
            }

            ctx->enemigos[e].color = listaMonstruos[idx].color;
            float escala = 1.0f + (lvl - 1) * 0.22f;
            ctx->enemigos[e].datos.maxHp = (int)(listaMonstruos[idx].baseHp * escala);
            ctx->enemigos[e].datos.hp    = ctx->enemigos[e].datos.maxHp;
            ctx->enemigos[e].datos.fue   = (int)(listaMonstruos[idx].baseAtk * escala);
            ctx->enemigos[e].datos.mag   = (int)(listaMonstruos[idx].baseMag * escala);
            ctx->enemigos[e].datos.def   = (int)(listaMonstruos[idx].baseDef * escala);
            ctx->enemigos[e].datos.defM  = (int)(listaMonstruos[idx].baseDef * 0.8f * escala);
            ctx->enemigos[e].datos.des   = (int)(listaMonstruos[idx].baseDes * escala);
            ctx->enemigos[e].expRecompensa = (int)(listaMonstruos[idx].baseExp * escala);
            ctx->enemigos[e].oroRecompensa = (int)(listaMonstruos[idx].baseOro * escala);

            ctx->totalExpCombate += ctx->enemigos[e].expRecompensa;
            ctx->totalOroCombate += ctx->enemigos[e].oroRecompensa;

            ctx->enemigos[e].medidorTurno = (float)(ctx->enemigos[e].datos.des * 2 + (rand() % 18));
            ctx->enemigos[e].animAtaque = 0;
            ctx->enemigos[e].timerHit = 0;
            ctx->enemigos[e].timerMuerte = 0;
        } else {
            ctx->enemigos[e].activo = false;
            ctx->enemigos[e].datos.hp = 0;
            ctx->enemigos[e].timerMuerte = 0;
        }
    }

    ctx->dmgPopups[3].activo = false;
    ctx->opcionMenu = 0;
    ctx->menuHabilidadesAbierto = false;
    ctx->opcionHabilidad = 0;
    ctx->verDetalleHabilidad = false;
    ctx->menuObjetosAbierto = false;
    ctx->opcionObjeto = 0;
    ctx->seleccionandoObjetivo = false;
    ctx->objetivoSeleccionado = 0;

    ctx->estado = ESTADO_COMBATE;
    ctx->jugador.defendiendo = false;
    ctx->animAtaqueJugador = 0;
    ctx->timerHitJugador = 0;
    ctx->expPendienteAnim = 0;
    ctx->oroGanadoFinal = 0;
    ctx->botinVictoria[0] = '\0';
    ctx->victoriaPendiente = false;
    ctx->timerDerrota = 0;

    ctx->medidorJugador = (float)(obtenerDesTotal(&ctx->jugador) * 2 + (rand() % 15));
    ctx->timerTurnoEnemigo = 42;
    proyectarColaTurnos(ctx);

    if (enMazmorra) {
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡Peligro en la Mazmorra! (%d rivales)", cantidadEnemigos);
    } else if (cantidadEnemigos == 1) {
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡Un %s te corta el paso!", ctx->enemigos[0].datos.nombre);
    } else {
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡Emboscada! Aparecen %d enemigos.", cantidadEnemigos);
    }
    snprintf(ctx->logLinea2, sizeof(ctx->logLinea2), "Prepara tu estrategia.");
}

static void prepararVictoria(ContextoJuego* ctx) {
    int sueTotal = obtenerSueTotal(&ctx->jugador);
    int oroExtra = (ctx->totalOroCombate * sueTotal * 2) / 100;
    ctx->oroGanadoFinal = ctx->totalOroCombate + oroExtra;
    ctx->jugador.oro += ctx->oroGanadoFinal;

    ctx->expPendienteAnim = ctx->totalExpCombate;
    ctx->infoLevelUp.nivelAnterior = ctx->jugador.nivel;
    ctx->infoLevelUp.nivelNuevo = ctx->jugador.nivel;
    ctx->infoLevelUp.incHp = 0;
    ctx->infoLevelUp.incMp = 0;
    ctx->infoLevelUp.incFue = 0;
    ctx->infoLevelUp.incDef = 0;
    ctx->infoLevelUp.incDes = 0;
    ctx->infoLevelUp.incInt = 0;
    ctx->infoLevelUp.incMag = 0;
    ctx->infoLevelUp.incDefM = 0;
    ctx->infoLevelUp.incSue = 0;
    ctx->infoLevelUp.nuevaHabilidad[0] = '\0';

    ctx->menuObjetosAbierto = false;
    ctx->menuHabilidadesAbierto = false;
    ctx->verDetalleHabilidad = false;
    ctx->seleccionandoObjetivo = false;

    int probDrop = 20 + sueTotal;
    if ((rand() % 100) < probDrop) {
        if (rand() % 2 == 0) {
            ctx->jugador.pociones++;
            snprintf(ctx->botinVictoria, sizeof(ctx->botinVictoria), "+1 Pocion obtenida");
        } else {
            ctx->jugador.eteres++;
            snprintf(ctx->botinVictoria, sizeof(ctx->botinVictoria), "+1 Eter obtenido");
        }
    } else {
        ctx->botinVictoria[0] = '\0';
    }

    ctx->estado = ESTADO_VICTORIA;
    snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡VICTORIA! Ganas +%d EXP y +%d Oro.",
             ctx->totalExpCombate, ctx->oroGanadoFinal);
    snprintf(ctx->logLinea2, sizeof(ctx->logLinea2), "Calculando experiencia obtenida...");
}

static void aplicarSubidaDeNivelProgresiva(ContextoJuego* ctx) {
    int habAntes = contarHabilidadesDesbloqueadas(ctx->jugador.nivel);
    ctx->jugador.nivel++;
    ctx->infoLevelUp.nivelNuevo = ctx->jugador.nivel;
    ctx->jugador.expSigNivel = (int)(ctx->jugador.expSigNivel * 1.45f);

    int incHp   = 18 + (rand() % 6);
    int incInt  = 2 + (rand() % 2);
    int incMp   = incInt * 4;
    int incFue  = 2 + (rand() % 3);
    int incDef  = 1 + (rand() % 3);
    int incDes  = 2 + (rand() % 3);
    int incMag  = 2 + (rand() % 3);
    int incDefM = 1 + (rand() % 3);
    int incSue  = 1 + (rand() % 3);

    ctx->infoLevelUp.incHp   += incHp;
    ctx->infoLevelUp.incMp   += incMp;
    ctx->infoLevelUp.incFue  += incFue;
    ctx->infoLevelUp.incDef  += incDef;
    ctx->infoLevelUp.incDes  += incDes;
    ctx->infoLevelUp.incInt  += incInt;
    ctx->infoLevelUp.incMag  += incMag;
    ctx->infoLevelUp.incDefM += incDefM;
    ctx->infoLevelUp.incSue  += incSue;

    ctx->jugador.maxHp += incHp;
    ctx->jugador.hp     = ctx->jugador.maxHp;
    ctx->jugador.intel += incInt;
    ctx->jugador.maxMp += incMp;
    ctx->jugador.mp     = ctx->jugador.maxMp;
    ctx->jugador.fue   += incFue;
    ctx->jugador.def   += incDef;
    ctx->jugador.des   += incDes;
    ctx->jugador.mag   += incMag;
    ctx->jugador.defM  += incDefM;
    ctx->jugador.sue   += incSue;

    int habDespues = contarHabilidadesDesbloqueadas(ctx->jugador.nivel);
    if (habDespues > habAntes) {
        snprintf(ctx->infoLevelUp.nuevaHabilidad, sizeof(ctx->infoLevelUp.nuevaHabilidad),
                 "¡Aprendes: %s!", LISTA_HABILIDADES[habDespues - 1].nombre);
    }
    audioReproducirSFX(SFX_POCION);
}

static void aplicarDanioEnemigo(ContextoJuego* ctx, int idxEne, int dmg, bool esCritico, u32 colorNormal) {
    if (idxEne < 0 || idxEne >= MAX_ENEMIGOS || !ctx->enemigos[idxEne].activo) return;
    ctx->enemigos[idxEne].datos.hp -= dmg;
    if (ctx->enemigos[idxEne].datos.hp <= 0) {
        ctx->enemigos[idxEne].datos.hp = 0;
        ctx->enemigos[idxEne].timerMuerte = 36;
    }
    ctx->enemigos[idxEne].timerHit = 18;

    float popupX = 232.0f + (idxEne * 46.0f);
    char bufDmg[20];
    if (esCritico) {
        snprintf(bufDmg, sizeof(bufDmg), "¡CRIT! -%d", dmg);
        lanzarPopup(&ctx->dmgPopups[idxEne], popupX - 10.0f, 108.0f, bufDmg, C2D_Color32(255, 110, 40, 255));
    } else {
        snprintf(bufDmg, sizeof(bufDmg), "-%d", dmg);
        lanzarPopup(&ctx->dmgPopups[idxEne], popupX, 112.0f, bufDmg, colorNormal);
    }
}

void confirmarObjetivoCombate(ContextoJuego* ctx) {
    int target = ctx->objetivoSeleccionado;
    if (target < 0 || !ctx->enemigos[target].activo || ctx->enemigos[target].datos.hp <= 0) {
        target = primerEnemigoVivo(ctx);
    }

    ctx->seleccionandoObjetivo = false;
    ctx->menuHabilidadesAbierto = false;
    ctx->verDetalleHabilidad = false;
    ctx->animAtaqueJugador = 24;

    bool critico = evaluarCritico(&ctx->jugador);

    if (ctx->accionPendienteTipo == 0) {
        audioReproducirSFX(SFX_GOLPE_JUGADOR);

        int dmg = obtenerFueTotal(&ctx->jugador) - (ctx->enemigos[target].datos.def / 2) + (rand() % 5);
        if (dmg < 1) dmg = 1;
        if (critico) dmg = (int)(dmg * 1.65f);

        aplicarDanioEnemigo(ctx, target, dmg, critico, C2D_Color32(255, 230, 80, 255));
        if (critico) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡GOLPE CRITICO a %s! %d DMG.", ctx->enemigos[target].datos.nombre, dmg);
        } else {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡Atacas a %s! Causas %d DMG.", ctx->enemigos[target].datos.nombre, dmg);
        }
    }
    else if (ctx->accionPendienteTipo == 1) {
        audioReproducirSFX(SFX_HABILIDAD_ESPADA);

        const Habilidad* hab = &LISTA_HABILIDADES[ctx->accionPendienteIndice];
        ctx->jugador.mp -= hab->costoMp;

        int fue = obtenerFueTotal(&ctx->jugador);
        int mag = obtenerMagTotal(&ctx->jugador);
        int defEne = ctx->enemigos[target].datos.def;
        int defMEne = ctx->enemigos[target].datos.defM;
        int dmg = 1;

        if (ctx->accionPendienteIndice == 0) {
            dmg = (int)(fue * 1.35f) - (defEne / 2) + (rand() % 4);
            ctx->medidorJugador += 35.0f;
        } else if (ctx->accionPendienteIndice == 2) {
            dmg = (int)(fue * 1.60f) + (rand() % 5);
        } else if (ctx->accionPendienteIndice == 4) {
            dmg = (int)((fue + mag) * 1.45f) - ((defEne + defMEne) / 3);
        }
        if (dmg < 1) dmg = 1;
        if (critico) dmg = (int)(dmg * 1.65f);

        aplicarDanioEnemigo(ctx, target, dmg, critico, C2D_Color32(100, 240, 255, 255));
        if (critico) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡%s CRITICO! %d DMG.", hab->nombre, dmg);
        } else {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡%s sobre %s! %d DMG.", hab->nombre, ctx->enemigos[target].datos.nombre, dmg);
        }
    }

    if (contarEnemigosVivos(ctx) == 0) {
        ctx->victoriaPendiente = true;
        return;
    }

    avanzarColaTurnosTrasAccion(ctx);
}

void seleccionarHabilidadCombate(ContextoJuego* ctx, int indiceHab) {
    int maxHab = contarHabilidadesDesbloqueadas(ctx->jugador.nivel);
    if (indiceHab < 0 || indiceHab >= maxHab) return;

    const Habilidad* hab = &LISTA_HABILIDADES[indiceHab];
    if (ctx->jugador.mp < hab->costoMp) {
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡PH insuficiente! Necesitas %d PH.", hab->costoMp);
        return;
    }

    if (hab->objetivo == OBJ_PROPIO) {
        audioReproducirSFX(SFX_POCION);

        ctx->jugador.mp -= hab->costoMp;
        int cura = 25 + (int)(obtenerMagTotal(&ctx->jugador) * 2.2f);
        int antes = ctx->jugador.hp;
        ctx->jugador.hp += cura;
        if (ctx->jugador.hp > ctx->jugador.maxHp) ctx->jugador.hp = ctx->jugador.maxHp;
        int realCurado = ctx->jugador.hp - antes;

        char bufCura[20];
        snprintf(bufCura, sizeof(bufCura), "+%d HP", realCurado);
        lanzarPopup(&ctx->dmgPopups[3], 68.0f, 110.0f, bufCura, C2D_Color32(80, 255, 140, 255));

        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡Lanzas %s! Recuperas +%d HP.", hab->nombre, realCurado);
        ctx->menuHabilidadesAbierto = false;
        ctx->verDetalleHabilidad = false;
        avanzarColaTurnosTrasAccion(ctx);
        return;
    }

    if (hab->objetivo == OBJ_TODOS_ENEMIGOS) {
        audioReproducirSFX(SFX_HABILIDAD_ESPADA);

        ctx->jugador.mp -= hab->costoMp;
        ctx->menuHabilidadesAbierto = false;
        ctx->verDetalleHabilidad = false;
        ctx->animAtaqueJugador = 24;

        int fue = obtenerFueTotal(&ctx->jugador);
        for (int e = 0; e < MAX_ENEMIGOS; e++) {
            if (ctx->enemigos[e].activo && ctx->enemigos[e].datos.hp > 0) {
                bool crit = evaluarCritico(&ctx->jugador);
                int dmg = (int)(fue * 1.25f) - (ctx->enemigos[e].datos.def / 2) + (rand() % 4);
                if (dmg < 1) dmg = 1;
                if (crit) dmg = (int)(dmg * 1.65f);
                aplicarDanioEnemigo(ctx, e, dmg, crit, C2D_Color32(255, 170, 50, 255));
            }
        }
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡%s golpea a todos los enemigos!", hab->nombre);

        if (contarEnemigosVivos(ctx) == 0) {
            ctx->victoriaPendiente = true;
            return;
        }
        avanzarColaTurnosTrasAccion(ctx);
        return;
    }

    ctx->accionPendienteTipo = 1;
    ctx->accionPendienteIndice = indiceHab;
    ctx->objetivoSeleccionado = primerEnemigoVivo(ctx);

    if (contarEnemigosVivos(ctx) == 1) {
        confirmarObjetivoCombate(ctx);
    } else {
        ctx->seleccionandoObjetivo = true;
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Elige objetivo con Izq/Der y pulsa [A].");
    }
}

void ejecutarComandoPrincipal(ContextoJuego* ctx, int comando) {
    if (ctx->colaTurnos[0] != TURNO_JUGADOR || ctx->victoriaPendiente || ctx->timerDerrota > 0) return;

    if (comando == 0) {
        ctx->accionPendienteTipo = 0;
        ctx->objetivoSeleccionado = primerEnemigoVivo(ctx);
        if (contarEnemigosVivos(ctx) == 1) {
            confirmarObjetivoCombate(ctx);
        } else {
            ctx->seleccionandoObjetivo = true;
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Elige objetivo con Izq/Der y pulsa [A].");
        }
    }
    else if (comando == 1) {
        ctx->menuHabilidadesAbierto = true;
        ctx->opcionHabilidad = 0;
        ctx->verDetalleHabilidad = false;
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Elige habilidad ([Y] para ver detalles).");
    }
    else if (comando == 2) {
        audioReproducirSFX(SFX_DEFENDER);
        ctx->jugador.defendiendo = true;
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Postura defensiva (-30%% DMG hasta tu turno).");
        avanzarColaTurnosTrasAccion(ctx);
    }
    else if (comando == 3) {
        ctx->menuObjetosAbierto = true;
        ctx->opcionObjeto = 0;
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Mochila abierta: elige un consumible.");
    }
    else if (comando == 4) {
        if ((rand() % 100) < 70) {
            ctx->estado = ESTADO_EXPLORACION;
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡Escapaste del combate con exito!");
            snprintf(ctx->logLinea2, sizeof(ctx->logLinea2), "Continua explorando el mapa.");
        } else {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡No pudiste escapar!");
            avanzarColaTurnosTrasAccion(ctx);
        }
    }
}

void usarObjetoCombate(ContextoJuego* ctx, int indiceObjeto) {
    if (ctx->colaTurnos[0] != TURNO_JUGADOR) return;

    if (indiceObjeto == 0) {
        if (ctx->jugador.pociones <= 0) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡No te quedan Pociones!");
            return;
        }
        if (ctx->jugador.hp >= ctx->jugador.maxHp) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Tu HP ya esta al maximo.");
            return;
        }

        audioReproducirSFX(SFX_POCION);
        ctx->jugador.pociones--;
        int antes = ctx->jugador.hp;
        ctx->jugador.hp += 45;
        if (ctx->jugador.hp > ctx->jugador.maxHp) ctx->jugador.hp = ctx->jugador.maxHp;
        int curado = ctx->jugador.hp - antes;

        char bufCura[20];
        snprintf(bufCura, sizeof(bufCura), "+%d HP", curado);
        lanzarPopup(&ctx->dmgPopups[3], 68.0f, 110.0f, bufCura, C2D_Color32(80, 255, 130, 255));

        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Usas Pocion: recuperas +%d HP.", curado);
        ctx->menuObjetosAbierto = false;
        avanzarColaTurnosTrasAccion(ctx);
    }
    else if (indiceObjeto == 1) {
        if (ctx->jugador.eteres <= 0) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡No te quedan Eteres!");
            return;
        }
        if (ctx->jugador.mp >= ctx->jugador.maxMp) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Tu PH ya esta al maximo.");
            return;
        }

        audioReproducirSFX(SFX_POCION);
        ctx->jugador.eteres--;
        int antes = ctx->jugador.mp;
        ctx->jugador.mp += 25;
        if (ctx->jugador.mp > ctx->jugador.maxMp) ctx->jugador.mp = ctx->jugador.maxMp;
        int curado = ctx->jugador.mp - antes;

        char bufMp[20];
        snprintf(bufMp, sizeof(bufMp), "+%d PH", curado);
        lanzarPopup(&ctx->dmgPopups[3], 68.0f, 110.0f, bufMp, C2D_Color32(180, 120, 255, 255));

        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Usas Eter: recuperas +%d PH.", curado);
        ctx->menuObjetosAbierto = false;
        avanzarColaTurnosTrasAccion(ctx);
    }
    else if (indiceObjeto == 2) {
        ctx->menuObjetosAbierto = false;
        snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "Elige un comando de accion.");
    }
}

void actualizarCombate(ContextoJuego* ctx) {
    if (ctx->estado == ESTADO_VICTORIA) {
        if (ctx->expPendienteAnim > 0) {
            int paso = (ctx->expPendienteAnim > 40) ? 3 : ((ctx->expPendienteAnim > 15) ? 2 : 1);
            if (paso > ctx->expPendienteAnim) paso = ctx->expPendienteAnim;

            ctx->jugador.exp += paso;
            ctx->expPendienteAnim -= paso;

            if (ctx->jugador.exp >= ctx->jugador.expSigNivel) {
                ctx->jugador.exp -= ctx->jugador.expSigNivel;
                aplicarSubidaDeNivelProgresiva(ctx);
            }

            if (ctx->expPendienteAnim == 0) {
                snprintf(ctx->logLinea2, sizeof(ctx->logLinea2), "Pulsa [A] o toca para continuar.");
            }
        }
        return;
    }

    if (ctx->estado != ESTADO_COMBATE) return;

    if (ctx->animAtaqueJugador > 0) ctx->animAtaqueJugador--;
    if (ctx->timerHitJugador > 0)   ctx->timerHitJugador--;

    bool algunEnemigoMuriendo = false;
    for (int e = 0; e < MAX_ENEMIGOS; e++) {
        if (ctx->enemigos[e].animAtaque > 0) ctx->enemigos[e].animAtaque--;
        if (ctx->enemigos[e].timerHit > 0)   ctx->enemigos[e].timerHit--;
        if (ctx->enemigos[e].timerMuerte > 0) {
            ctx->enemigos[e].timerMuerte--;
            algunEnemigoMuriendo = true;
        }
    }

    for (int i = 0; i < 4; i++) {
        if (ctx->dmgPopups[i].activo) {
            ctx->dmgPopups[i].y -= 0.65f;
            ctx->dmgPopups[i].timer--;
            if (ctx->dmgPopups[i].timer <= 0) ctx->dmgPopups[i].activo = false;
        }
    }

    if (ctx->victoriaPendiente) {
        if (!algunEnemigoMuriendo) {
            ctx->victoriaPendiente = false;
            prepararVictoria(ctx);
        }
        return;
    }

    if (ctx->timerDerrota > 0) {
        ctx->timerDerrota--;
        if (ctx->timerDerrota == 0) {
            ctx->jugador.hp = ctx->jugador.maxHp;
            ctx->jugador.mp = ctx->jugador.maxMp;
            ctx->jugador.exp /= 2;
            ctx->jugador.oro /= 2;
            ctx->playerX = 140.0f;
            ctx->playerY = 230.0f;
            ctx->menuObjetosAbierto = false;
            ctx->menuHabilidadesAbierto = false;
            ctx->verDetalleHabilidad = false;
            ctx->seleccionandoObjetivo = false;
            ctx->estado = ESTADO_EXPLORACION;
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡Has caido! Despiertas en el Pueblo.");
            snprintf(ctx->logLinea2, sizeof(ctx->logLinea2), "Pierdes la mitad de tu EXP y Oro.");
        }
        return;
    }

    if (ctx->colaTurnos[0] >= TURNO_ENEMIGO_0) {
        int idxEne = ctx->colaTurnos[0] - TURNO_ENEMIGO_0;

        if (!ctx->enemigos[idxEne].activo || ctx->enemigos[idxEne].datos.hp <= 0) {
            avanzarColaTurnosTrasAccion(ctx);
            return;
        }

        if (ctx->timerTurnoEnemigo > 0) {
            ctx->timerTurnoEnemigo--;
            return;
        }

        int idxPlantilla = plantillaEnemigoActual[idxEne];
        bool lanzaMagia = ((rand() % 100) < listaMonstruos[idxPlantilla].probMagia);

        int dmgBruto = 1;
        if (lanzaMagia) {
            dmgBruto = (int)(ctx->enemigos[idxEne].datos.mag * 1.15f) - (obtenerDefMTotal(&ctx->jugador) / 2) + (rand() % 5);
        } else {
            dmgBruto = ctx->enemigos[idxEne].datos.fue - (obtenerDefTotal(&ctx->jugador) / 2) + (rand() % 4);
        }
        if (dmgBruto < 1) dmgBruto = 1;

        int dmgFinal = ctx->jugador.defendiendo ? (int)(dmgBruto * 0.70f) : dmgBruto;
        if (dmgFinal < 1) dmgFinal = 1;

        ctx->jugador.hp -= dmgFinal;
        ctx->enemigos[idxEne].animAtaque = 16;
        ctx->timerHitJugador = 18;
        audioReproducirSFX(lanzaMagia ? SFX_HABILIDAD_ESPADA : SFX_GOLPE_ENEMIGO);

        char bufDmg[24];
        u32 colorPopupDmg;
        if (lanzaMagia) {
            snprintf(bufDmg, sizeof(bufDmg), "-%d (MAG)", dmgFinal);
            colorPopupDmg = ctx->jugador.defendiendo ? C2D_Color32(130, 210, 255, 255) : C2D_Color32(210, 110, 255, 255);
        } else {
            if (ctx->jugador.defendiendo) snprintf(bufDmg, sizeof(bufDmg), "-%d (BLK)", dmgFinal);
            else snprintf(bufDmg, sizeof(bufDmg), "-%d", dmgFinal);
            colorPopupDmg = ctx->jugador.defendiendo ? C2D_Color32(100, 230, 255, 255) : C2D_Color32(255, 80, 80, 255);
        }

        lanzarPopup(&ctx->dmgPopups[3], 68.0f, 110.0f, bufDmg, colorPopupDmg);

        if (ctx->jugador.hp <= 0) {
            ctx->jugador.hp = 0;
            ctx->timerDerrota = 55;
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡%s ha caido en combate!", ctx->jugador.nombre);
            snprintf(ctx->logLinea2, sizeof(ctx->logLinea2), "Tu vision se desvanece...");
            return;
        }

        if (lanzaMagia) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡%s usa %s! Recibes %d DMG magico.",
                    ctx->enemigos[idxEne].datos.nombre, listaMonstruos[idxPlantilla].nombreHechizo, dmgFinal);
        } else if (ctx->jugador.defendiendo) {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "%s ataca: ¡Bloqueas! Recibes %d DMG.", ctx->enemigos[idxEne].datos.nombre, dmgFinal);
        } else {
            snprintf(ctx->logLinea1, sizeof(ctx->logLinea1), "¡%s ataca! Recibes %d DMG.", ctx->enemigos[idxEne].datos.nombre, dmgFinal);
        }

        avanzarColaTurnosTrasAccion(ctx);
    }
}