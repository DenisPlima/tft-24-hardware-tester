#include "state.h"

uint16_t g_testsExecuted = 0;
TestRecord g_results[TOTAL_TESTS + 1];

void stateRecordResult(uint8_t testNumber, const char *name, TestResult result, unsigned long durationMs) {
  if (testNumber < 1 || testNumber > TOTAL_TESTS) return;
  g_results[testNumber].name = name;
  g_results[testNumber].result = result;
  g_results[testNumber].durationMs = durationMs;
  g_testsExecuted++;
}
