// Generado por /workspace/enemigos/exportar.py - no editar a mano
#pragma once
// Cuadros en gfx/enemigos.t3s, indice = id*20 + cuadro:
// idle 0-3, ataque 4-9, herido 10-13, muerte 14-17, break 18-19.
// Ancla = pies. Enemigos ya volteados (miran a la izquierda, hacia el heroe).
// alcance[6][2] = punta (o hocico) relativa al ancla en cada cuadro de ataque (x negativo).
typedef struct {
    int w, h, ax, ay;
    int alcance[6][2];
    int tiene_break;
    int hit_cuadro;
    int es_rango;
    int lunge;
} MetaEnemigo;

#define NUM_ENEMIGOS 3
#define ENE_CUADROS  20
enum { EC_IDLE = 0, EC_ATAQUE = 4, EC_HERIDO = 10, EC_MUERTE = 14, EC_BREAK = 18 };

static const MetaEnemigo META_ENEMIGO[NUM_ENEMIGOS] = {
    {88, 95, 44, 94, {{-38, -45}, {-37, -46}, {-38, -47}, {-34, -43}, {-33, -25}, {-37, -44}}, 1, 3, 1, 8}, // rata_arquera (rata_arquera.webp+rata_arquera_break.png)
    {83, 89, 42, 88, {{-25, -27}, {-26, -18}, {-35, -4}, {-42, -4}, {-35, -10}, {-25, -28}}, 1, 3, 0, 70}, // cabra (cabra.webp+cabra_break.png)
    {96, 96, 47, 95, {{-42, -39}, {-38, -55}, {-36, -46}, {-47, -2}, {-39, -3}, {-41, -38}}, 1, 3, 0, 42}, // sapo (sapo.webp+sapo_break.png)
};
