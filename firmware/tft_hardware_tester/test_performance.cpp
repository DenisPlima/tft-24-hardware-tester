#include "test_performance.h"

bool testPerformance() {
  Serial.println(F("== TESTE 12: DESEMPENHO =="));
  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 12: DESEMPENHO");

  uint32_t opCount = 0;

  // 1) Tempo medio para preencher a tela inteira (media de 5 execucoes)
  uint32_t t0 = micros();
  for (uint8_t i = 0; i < 5; i++) {
    tft.setColor((i % 2) ? COLOR_BLACK : COLOR_BLUE);
    tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    opCount++;
  }
  uint32_t fillTimeUs = (micros() - t0) / 5;
  float fps = (fillTimeUs > 0) ? (1000000.0f / (float)fillTimeUs) : 0.0f;

  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);

  // 2) 100 linhas
  t0 = micros();
  for (uint16_t i = 0; i < 100; i++) {
    int16_t x1 = (i * 7) % DISPLAY_WIDTH;
    int16_t y1 = 23 + (i * 3) % (DISPLAY_HEIGHT - 23);
    int16_t x2 = (i * 13) % DISPLAY_WIDTH;
    int16_t y2 = 23 + (i * 11) % (DISPLAY_HEIGHT - 23);
    tft.setColor(COLOR_GREEN);
    tft.drawLine(x1, y1, x2, y2);
    opCount++;
  }
  uint32_t linesTimeUs = micros() - t0;

  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);

  // 3) 100 circulos
  t0 = micros();
  for (uint16_t i = 0; i < 100; i++) {
    int16_t cx = 20 + (i * 7) % (DISPLAY_WIDTH - 40);
    int16_t cy = 40 + (i * 5) % (DISPLAY_HEIGHT - 60);
    tft.setColor(COLOR_MAGENTA);
    tft.drawCircle(cx, cy, 8);
    opCount++;
  }
  uint32_t circlesTimeUs = micros() - t0;

  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);

  // 4) Atualizacao de texto (50 ciclos de limpar+escrever)
  tft.setFont(BigFont);
  t0 = micros();
  for (uint16_t i = 0; i < 50; i++) {
    tft.setColor(COLOR_BLACK);
    tft.fillRect(10, 60, 230, 80);
    tft.setColor(COLOR_WHITE);
    tft.setBackColor(COLOR_BLACK);
    char buf[16];
    snprintf(buf, sizeof(buf), "Contador: %u", i);
    tft.print(buf, 10, 60);
    opCount += 2;
  }
  uint32_t textTimeUs = micros() - t0;

  // ---- Exibe resultados no display ----
  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 12: RESULTADOS");
  tft.setFont(SmallFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLACK);

  int16_t y = 28;
  char buf[32];
  snprintf(buf, sizeof(buf), "Fill tela: %ld ms", fillTimeUs / 1000);
  tft.print(buf, 4, y); y += 12;
  snprintf(buf, sizeof(buf), "FPS aprox: %d", (int)fps);
  tft.print(buf, 4, y); y += 12;
  snprintf(buf, sizeof(buf), "100 linhas: %ld ms", linesTimeUs / 1000);
  tft.print(buf, 4, y); y += 12;
  snprintf(buf, sizeof(buf), "100 circulos: %ld ms", circlesTimeUs / 1000);
  tft.print(buf, 4, y); y += 12;
  snprintf(buf, sizeof(buf), "50x atualiz texto: %ld ms", textTimeUs / 1000);
  tft.print(buf, 4, y); y += 12;
  snprintf(buf, sizeof(buf), "Operacoes: %lu", (unsigned long)opCount);
  tft.print(buf, 4, y);

  // ---- Exibe resultados no Serial ----
  Serial.print(F("Fill tela (media): ")); Serial.print(fillTimeUs); Serial.println(F(" us"));
  Serial.print(F("FPS aproximado: ")); Serial.println(fps, 2);
  Serial.print(F("100 linhas: ")); Serial.print(linesTimeUs); Serial.println(F(" us"));
  Serial.print(F("100 circulos: ")); Serial.print(circlesTimeUs); Serial.println(F(" us"));
  Serial.print(F("50x atualizacao de texto: ")); Serial.print(textTimeUs); Serial.println(F(" us"));
  Serial.print(F("Total de operacoes de desenho: ")); Serial.println(opCount);

  bool pass = (fillTimeUs > 0 && linesTimeUs > 0 && circlesTimeUs > 0 && textTimeUs > 0);
  Serial.println(pass ? F("Resultado: PASS") : F("Resultado: FAIL"));
  Serial.println();
  return pass;
}
