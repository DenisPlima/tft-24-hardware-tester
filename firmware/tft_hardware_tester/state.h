// ============================================================================
// state.h - Estado global compartilhado entre os modulos de teste
// ============================================================================
#ifndef STATE_H
#define STATE_H

#include <Arduino.h>
#include <UTFT.h>
#include "config.h"

extern UTFT tft;

// Nome do controlador - confirmado em bancada (ver config.h), fixo porque a
// UTFT nao oferece leitura de ID do controlador (biblioteca somente-escrita).
#define CONTROLLER_NAME "SSD1289"

// Contador de testes executados nesta sessao (requisito do menu principal)
extern uint16_t g_testsExecuted;

// Historico de resultados (index = numero do teste, 1..TOTAL_TESTS)
extern TestRecord g_results[TOTAL_TESTS + 1];

// Registra o resultado de um teste (atualiza historico + contador)
void stateRecordResult(uint8_t testNumber, const char *name, TestResult result, unsigned long durationMs);

#endif // STATE_H
