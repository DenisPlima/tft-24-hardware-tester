// ============================================================================
// ui.h - Helpers de interface grafica compartilhados entre os testes
// ============================================================================
#ifndef UI_H
#define UI_H

#include <UTFT.h>
#include "config.h"

// Objeto global definido em tft_hardware_tester.ino
extern UTFT tft;

// Fontes padrao da UTFT (definidas em DefaultFonts.c, dentro da biblioteca)
extern uint8_t BigFont[];
extern uint8_t SmallFont[];

// Desenha a barra de titulo padrao no topo da tela
void uiDrawHeader(const char *title);

// Desenha um "botao" de voltar grande (touch-friendly) no rodape
void uiDrawBackButton();

// Retangulo do botao "voltar" (para deteccao de toque)
extern const int16_t UI_BACK_BTN_X;
extern const int16_t UI_BACK_BTN_Y;
extern const int16_t UI_BACK_BTN_W;
extern const int16_t UI_BACK_BTN_H;

// Mostra um banner grande de resultado PASS/FAIL centralizado
void uiShowResultBanner(bool pass);

// Limpa a area de conteudo (abaixo do header)
void uiClearContent();

// Espera N milissegundos (delay simples, mantido curto em cada chamada)
void uiWaitMs(unsigned long ms);

#endif // UI_H
