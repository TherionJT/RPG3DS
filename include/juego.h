#ifndef JUEGO_H
#define JUEGO_H

#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>

#define MAPA_ANCHO 1200.0f
#define MAPA_ALTO  720.0f

#define MAX_ENEMIGOS    3
#define MAX_HABILIDADES 5
#define MAX_ARMAS       8
#define MAX_ARMADURAS   8

typedef enum {
    ESTADO_EXPLORACION = 0,
    ESTADO_COMBATE,
    ESTADO_VICTORIA,
    ESTADO_LEVEL_UP,
    ESTADO_TIENDA,
    ESTADO_EQUIPO
} EstadoJuego;

typedef enum {
    TURNO_JUGADOR = 0,
    TURNO_ENEMIGO_0,
    TURNO_ENEMIGO_1,
    TURNO_ENEMIGO_2
} TipoTurno;

typedef enum {
    OBJ_UN_ENEMIGO = 0,
    OBJ_TODOS_ENEMIGOS,
    OBJ_PROPIO
} TipoObjetivo;

typedef struct {
    const char* nombre;
    int nivelReq;
    int costoMp;
    bool esMagia;
    TipoObjetivo objetivo;
    const char* descCorta;
    const char* descDetallada;
} Habilidad;

typedef struct {
    const char* nombre;
    int bonoFue;
    int bonoMag;
    int precio;
    int filaIcono;
    int colIcono;
} InfoArma;

typedef struct {
    const char* nombre;
    int bonoDef;
    int bonoDes;
    int precio;
    int filaIcono;
    int colIcono;
} InfoArmadura;

extern const InfoArma CATALOGO_ARMAS[MAX_ARMAS];
extern const InfoArmadura CATALOGO_ARMADURAS[MAX_ARMADURAS];

typedef struct {
    int idArma;
    int idArmadura;
    char accesorio[28];
    int bonoFue;
    int bonoDef;
    int bonoDes;
    int bonoMag;
    int bonoDefM;
    int bonoSue;
} Equipamiento;

typedef struct {
    char nombre[28];
    int nivel;
    int exp;
    int expSigNivel;
    int oro;
    int pociones;
    int eteres;

    int hp;
    int maxHp;
    int mp;
    int maxMp;
    int fue;
    int def;
    int des;
    int intel;
    int mag;
    int defM;
    int sue;

    bool defendiendo;
    Equipamiento equipo;
} Entidad;

typedef struct {
    bool activo;
    Entidad datos;
    u32 color;
    int expRecompensa;
    int oroRecompensa;
    float medidorTurno;
    int animAtaque;
    int timerHit;
    int timerMuerte;
} EnemigoCombate;

typedef struct {
    bool activo;
    float x;
    float y;
    int timer;
    u32 color;
    char texto[24];
} TextoFlotante;

typedef struct {
    int nivelAnterior;
    int nivelNuevo;
    int incHp;
    int incMp;
    int incFue;
    int incDef;
    int incDes;
    int incInt;
    int incMag;
    int incDefM;
    int incSue;
    char nuevaHabilidad[48];
} InfoSubidaNivel;

typedef struct {
    EstadoJuego estado;
    int frameGlobal;

    float playerX;
    float playerY;
    float distanciaEncuentro;
    int cooldownEncuentro;
    bool caminando;
    bool mirandoIzquierda;
    int zonaActual;
    int edificioCercano;

    Entidad jugador;
    EnemigoCombate enemigos[MAX_ENEMIGOS];

    float medidorJugador;
    TipoTurno colaTurnos[6];
    int timerTurnoEnemigo;

    int opcionMenu;
    bool menuHabilidadesAbierto;
    int opcionHabilidad;
    bool verDetalleHabilidad;
    bool menuObjetosAbierto;
    int opcionObjeto;
    int opcionTienda;

    bool seleccionandoObjetivo;
    int objetivoSeleccionado;
    int accionPendienteTipo;
    int accionPendienteIndice;

    int animAtaqueJugador;
    int timerHitJugador;
    TextoFlotante dmgPopups[4];

    int totalExpCombate;
    int totalOroCombate;
    int expPendienteAnim;
    int oroGanadoFinal;
    char botinVictoria[36];
    bool victoriaPendiente;
    int timerDerrota;

    InfoSubidaNivel infoLevelUp;

    char logLinea1[64];
    char logLinea2[64];
} ContextoJuego;

#endif