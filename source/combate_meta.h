// Generado por /workspace/combate/export.py - no editar a mano
#pragma once
// Cuadros por viajero en gfx/combate_<id>.t3s (18): idle 0-3, ataque 4-9, defensa 10-11,
// herido 12-13, victoria 14-15, KO 16-17. Ancla = pies. alcance = punta del arma por cuadro de ataque.
typedef struct { int w, h, ax, ay; int alcance[6][2]; int real; } MetaCombate;
static const MetaCombate META_COMBATE[8] = {
    {80, 64, 37, 63, {{17, -54}, {23, -52}, {37, -40}, {42, -40}, {33, -32}, {17, -53}}, 1}, // ophilia (ophilia_combate.png)
    {66, 61, 31, 60, {{13, -33}, {20, -33}, {34, -35}, {19, -37}, {15, -33}, {12, -32}}, 1}, // cyrus (cyrus_combate.png)
    {77, 61, 34, 60, {{28, -27}, {33, -39}, {38, -26}, {42, -26}, {31, -32}, {29, -28}}, 1}, // tressa (tressa_combate.png)
    {84, 68, 37, 67, {{20, -17}, {24, -3}, {17, -2}, {46, -15}, {38, -6}, {22, -2}}, 1}, // olberic (olberic_combate.png)
    {74, 61, 37, 60, {{21, -2}, {23, -21}, {26, -16}, {36, -31}, {24, -8}, {21, -3}}, 1}, // primrose (primrose_combate.png)
    {70, 64, 38, 63, {{18, -16}, {16, -37}, {16, -24}, {27, -2}, {30, -4}, {19, -4}}, 1}, // alfyn (alfyn_combate.png)
    {69, 66, 36, 65, {{20, -16}, {24, -16}, {27, -29}, {30, -20}, {19, -16}, {18, -13}}, 1}, // therion (therion_combate.png)
    {65, 61, 35, 60, {{21, -35}, {22, -33}, {29, -35}, {25, -32}, {22, -34}, {21, -33}}, 1}, // haanit (haanit_combate.png)
};
#define FX_TAJO_0 0
#define FX_TAJO_1 1
#define FX_TAJO_2 2
#define FX_ESTOCADA_0 3
#define FX_ESTOCADA_1 4
#define FX_IMPACTO_0 5
#define FX_IMPACTO_1 6
#define FX_IMPACTO_2 7
#define FX_FLECHA 8
#define FX_ORBE_0 9
#define FX_ORBE_1 10
#define FX_LUZ_0 11
#define FX_LUZ_1 12
#define FX_LUZ_2 13
#define FX_LUZ_3 14
#define FX_BRILLO_0 15
#define FX_BRILLO_1 16
#define FX_BRILLO_2 17
#define FX_ESCUDO_0 18
#define FX_ESCUDO_1 19
#define FX_GARRA_0 20
#define FX_GARRA_1 21
