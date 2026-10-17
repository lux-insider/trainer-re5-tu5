// As trapaças do Resident Evil 5 TU5 (Xbox 360, Title ID 434307D4).
//
// Cada trapaça troca uma ou duas instruções do default.xex por nop
// (0x60000000): a instrução que gravaria o valor (vida, munição...) deixa de
// rodar e o valor fica parado. Desligar grava a instrução original de volta.
//
// Endereços e instruções originais conferidos contra o default.xex da TU5
// traduzida.
// São 12 endereços. O trainer só grava se encontrar exatamente estas
// instruções (ou o nop).
#pragma once

#include "xdk_extra.h"

#define INSTRUCAO_NOP   0x60000000

struct Instrucao {
    DWORD endereco;
    DWORD original;
};

struct Trapaca {
    PCWSTR nome;
    PCWSTR botoes;          // como aparece na lista de códigos
    WORD   codigo[4];       // os 4 botões da caixa de código (VK_PAD_*)
    int    quantas;         // instruções usadas (1 ou 2)
    Instrucao instrucoes[2];
};

static const Trapaca TRAPACAS[] = {
    { L"Vida infinita",          L"RT RT RT RT",
      { VK_PAD_RTRIGGER, VK_PAD_RTRIGGER, VK_PAD_RTRIGGER, VK_PAD_RTRIGGER },
      2, { { 0x8269A8CC, 0xB0DF12E4 }, { 0x8269A8DC, 0xB14B0004 } } },
    { L"Munição infinita",   L"LT LT LT LT",
      { VK_PAD_LTRIGGER, VK_PAD_LTRIGGER, VK_PAD_LTRIGGER, VK_PAD_LTRIGGER },
      1, { { 0x82766954, 0x91440008 } } },
    { L"Granadas infinitas",     L"RB RB RB RB",
      { VK_PAD_RSHOULDER, VK_PAD_RSHOULDER, VK_PAD_RSHOULDER, VK_PAD_RSHOULDER },
      1, { { 0x823096B4, 0x93B80008 } } },
    { L"Minas infinitas",        L"X X X X",
      { VK_PAD_X, VK_PAD_X, VK_PAD_X, VK_PAD_X },
      1, { { 0x82308F44, 0x937E0008 } } },
    { L"Dinheiro infinito",      L"LB LB LB LB",
      { VK_PAD_LSHOULDER, VK_PAD_LSHOULDER, VK_PAD_LSHOULDER, VK_PAD_LSHOULDER },
      2, { { 0x828007A0, 0x916300D4 }, { 0x828007B0, 0x816300D4 } } },
    { L"Pontos do Mercenaries",  L"Y Y Y Y",
      { VK_PAD_Y, VK_PAD_Y, VK_PAD_Y, VK_PAD_Y },
      2, { { 0x8224125C, 0x90680414 }, { 0x822394A8, 0x938A0414 } } },
    { L"Pontos das DLCs",        L"LB RB LB RB",
      { VK_PAD_LSHOULDER, VK_PAD_RSHOULDER, VK_PAD_LSHOULDER, VK_PAD_RSHOULDER },
      2, { { 0x826919E4, 0x93DF2BCC }, { 0x8230B264, 0x917F2BCC } } },
    { L"Relógio parado",          L"LT RT LT RT",
      { VK_PAD_LTRIGGER, VK_PAD_RTRIGGER, VK_PAD_LTRIGGER, VK_PAD_RTRIGGER },
      1, { { 0x82239A14, 0xD01D04E8 } } },
};

static const int NUM_TRAPACAS = sizeof(TRAPACAS) / sizeof(TRAPACAS[0]);

// Códigos especiais
static const WORD CODIGO_TODAS[4] = { VK_PAD_DPAD_UP,   VK_PAD_DPAD_UP,   VK_PAD_DPAD_UP,   VK_PAD_DPAD_UP };
static const WORD CODIGO_LISTA[4] = { VK_PAD_DPAD_DOWN, VK_PAD_DPAD_DOWN, VK_PAD_DPAD_DOWN, VK_PAD_DPAD_DOWN };
