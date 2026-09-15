// ============================================================================
// test_touch.h - Teste 11 (touch resistivo, controlador XPT2046)
// ============================================================================
//
// Este teste SO existe de fato se TOUCH_HARDWARE_PRESENT estiver definido em
// config.h. Caso contrario, testTouch() apenas informa
// "TOUCH NAO DISPONIVEL NESTE HARDWARE" e retorna - nenhuma leitura de touch
// fictícia é feita quando o recurso está desabilitado.
//
// A leitura NAO usa o pino de IRQ (T_IRQ/D2) porque ele nao respondeu em
// bancada neste shield (ver config.h). A leitura e feita via SPI manual
// (bit-bang) continuo, filtrando leituras invalidas pelos extremos de
// escala do ADC.
// ============================================================================
#ifndef TEST_TOUCH_H
#define TEST_TOUCH_H

#include "config.h"
#include "state.h"
#include "ui.h"

bool testTouch();

// Inicializa os pinos do touch. Chamar uma vez em setup().
void touchInit();

// Leitura rapida usada pelo menu principal (fora do Teste 11): retorna
// true e preenche (sx,sy) em coordenadas de tela se houve um toque valido
// ja filtrado/debounced. Retorna sempre false se o touch estiver desativado.
bool touchPoll(int16_t *sx, int16_t *sy);

#endif // TEST_TOUCH_H
