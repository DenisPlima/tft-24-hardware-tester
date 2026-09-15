#include "test_touch.h"

#if TOUCH_HARDWARE_PRESENT
#include <EEPROM.h>

struct RawTouch { int16_t x, y, z; bool valid; };
struct CalibData { int16_t rawX0, rawY0, rawX1, rawY1; };
static CalibData g_cal = {300, 300, 3800, 3800}; // fallback, sobrescrito pela calibracao
static const int16_t CAL_MARGIN = 30;

// ----------------------------------------------------------------------------
// Leitura SPI manual (bit-bang) do controlador de touch XPT2046. Nao usa o
// pino de IRQ (nao responde neste shield - ver config.h). Validada em
// bancada: os valores brutos mudam de forma consistente ao tocar a tela.
// ----------------------------------------------------------------------------
static uint16_t xpt2046ReadRaw(uint8_t cmd) {
  digitalWrite(TOUCH_CS, LOW);
  for (int8_t i = 7; i >= 0; i--) {
    digitalWrite(TOUCH_DIN, (cmd >> i) & 1);
    digitalWrite(TOUCH_CLK, LOW);
    delayMicroseconds(2);
    digitalWrite(TOUCH_CLK, HIGH);
    delayMicroseconds(2);
  }
  digitalWrite(TOUCH_CLK, LOW);
  uint16_t result = 0;
  for (int8_t i = 15; i >= 0; i--) {
    digitalWrite(TOUCH_CLK, HIGH);
    delayMicroseconds(2);
    digitalWrite(TOUCH_CLK, LOW);
    delayMicroseconds(2);
    result <<= 1;
    if (digitalRead(TOUCH_DOUT)) result |= 1;
  }
  digitalWrite(TOUCH_CS, HIGH);
  return result >> 4; // 12 bits uteis
}

static RawTouch readTouchRaw() {
  RawTouch t;
  // Canais trocados em relacao ao datasheet do XPT2046: em bancada, o eixo
  // fisico do filme resistivo deste painel fica a 90 graus do conteudo
  // landscape mostrado pelo controlador (comum quando o filme e nativamente
  // portrait e a UTFT gira so o conteudo, nao o touch). 0x90 alimenta X,
  // 0xD0 alimenta Y - o oposto do uso "de livro".
  t.x = xpt2046ReadRaw(0x90);
  t.y = xpt2046ReadRaw(0xD0);
  // Z1 como indicador aproximado de pressao (nao calibrado com a
  // resistencia real do painel - serve apenas como referencia relativa).
  t.z = xpt2046ReadRaw(0xB0);
  t.valid = (t.x > TOUCH_RAW_MIN_VALID && t.x < TOUCH_RAW_MAX_VALID &&
             t.y > TOUCH_RAW_MIN_VALID && t.y < TOUCH_RAW_MAX_VALID);
  return t;
}

// Filtro simples (mediana de 3 amostras) + debounce por tempo.
static bool readTouchFiltered(RawTouch *out) {
  static unsigned long lastAcceptedMs = 0;

  RawTouch samples[3];
  for (uint8_t i = 0; i < 3; i++) samples[i] = readTouchRaw();

  uint8_t validCount = 0;
  for (uint8_t i = 0; i < 3; i++) if (samples[i].valid) validCount++;
  if (validCount < 2) return false; // exige pelo menos 2 de 3 leituras validas

  // Mediana simples por X (insertion sort)
  for (uint8_t i = 1; i < 3; i++) {
    RawTouch key = samples[i];
    int8_t j = i - 1;
    while (j >= 0 && samples[j].x > key.x) { samples[j + 1] = samples[j]; j--; }
    samples[j + 1] = key;
  }
  RawTouch med = samples[1];
  if (!med.valid) return false;

  unsigned long now = millis();
  if (now - lastAcceptedMs < TOUCH_DEBOUNCE_MS) return false;
  lastAcceptedMs = now;

  *out = med;
  return true;
}

// ----------------------------------------------------------------------------
// Calibracao (persistida em EEPROM)
// ----------------------------------------------------------------------------
static void loadCalibration() {
  uint16_t magic = 0;
  EEPROM.get(EEPROM_CAL_MAGIC_ADDR, magic);
  if (magic == EEPROM_CAL_MAGIC_VALUE) {
    EEPROM.get(EEPROM_CAL_DATA_ADDR, g_cal);
  }
}

static void saveCalibration() {
  uint16_t magic = EEPROM_CAL_MAGIC_VALUE;
  EEPROM.put(EEPROM_CAL_MAGIC_ADDR, magic);
  EEPROM.put(EEPROM_CAL_DATA_ADDR, g_cal);
}

static void mapRawToScreen(const RawTouch &t, int16_t *sx, int16_t *sy) {
  int16_t x = map(t.x, g_cal.rawX0, g_cal.rawX1, CAL_MARGIN, DISPLAY_WIDTH - CAL_MARGIN);
  int16_t y = map(t.y, g_cal.rawY0, g_cal.rawY1, CAL_MARGIN, DISPLAY_HEIGHT - CAL_MARGIN);
  *sx = constrain(x, 0, DISPLAY_WIDTH - 1);
  *sy = constrain(y, 0, DISPLAY_HEIGHT - 1);
}

static void drawCrosshair(int16_t x, int16_t y, uint16_t color) {
  tft.setColor(color);
  tft.drawLine(x - 10, y, x + 10, y);
  tft.drawLine(x, y - 10, x, y + 10);
  tft.drawCircle(x, y, 6);
}

// Espera um toque valido (ou cancelamento via Serial 's'/'S').
static bool waitForTouch(RawTouch *result, unsigned long timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    RawTouch t;
    if (readTouchFiltered(&t)) { *result = t; return true; }
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 's' || c == 'S') return false;
    }
  }
  return false;
}

static bool runCalibration() {
  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 11: CALIBRACAO");
  tft.setFont(SmallFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLACK);
  tft.print((char *)"Toque no alvo (1/2). 's' no Serial pula.", 4, 26);
  int16_t x0 = CAL_MARGIN, y0 = CAL_MARGIN + 20;
  drawCrosshair(x0, y0, COLOR_YELLOW);
  Serial.println(F("Calibracao - toque no alvo superior-esquerdo."));

  RawTouch p0;
  if (!waitForTouch(&p0, 20000)) {
    Serial.println(F("Calibracao cancelada ou sem resposta (timeout)."));
    return false;
  }
  drawCrosshair(x0, y0, COLOR_GREEN);
  uiWaitMs(300);

  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 11: CALIBRACAO");
  tft.setFont(SmallFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLACK);
  tft.print((char *)"Toque no alvo (2/2).", 4, 26);
  int16_t x1 = DISPLAY_WIDTH - CAL_MARGIN, y1 = DISPLAY_HEIGHT - CAL_MARGIN;
  drawCrosshair(x1, y1, COLOR_YELLOW);
  Serial.println(F("Calibracao - toque no alvo inferior-direito."));

  RawTouch p1;
  if (!waitForTouch(&p1, 20000)) {
    Serial.println(F("Calibracao cancelada ou sem resposta (timeout)."));
    return false;
  }
  drawCrosshair(x1, y1, COLOR_GREEN);
  uiWaitMs(300);

  g_cal.rawX0 = p0.x; g_cal.rawY0 = p0.y;
  g_cal.rawX1 = p1.x; g_cal.rawY1 = p1.y;
  saveCalibration();

  Serial.print(F("Calibracao OK: rawX0=")); Serial.print(g_cal.rawX0);
  Serial.print(F(" rawY0=")); Serial.print(g_cal.rawY0);
  Serial.print(F(" rawX1=")); Serial.print(g_cal.rawX1);
  Serial.print(F(" rawY1=")); Serial.println(g_cal.rawY1);
  return true;
}

// ----------------------------------------------------------------------------
// Teste de toque nos 4 cantos
// ----------------------------------------------------------------------------
static bool cornerTouchTest() {
  int16_t corners[4][2] = {{14, 30}, {DISPLAY_WIDTH - 14, 30}, {14, DISPLAY_HEIGHT - 14}, {DISPLAY_WIDTH - 14, DISPLAY_HEIGHT - 14}};
  const char *labels[4] = {"SUP-ESQ", "SUP-DIR", "INF-ESQ", "INF-DIR"};
  bool allOk = true;

  for (uint8_t i = 0; i < 4; i++) {
    tft.setColor(COLOR_BLACK);
    tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    uiDrawHeader("TESTE 11: CANTOS");
    tft.setFont(SmallFont);
    tft.setColor(COLOR_WHITE);
    tft.setBackColor(COLOR_BLACK);
    char buf[24];
    snprintf(buf, sizeof(buf), "Toque no canto: %s", labels[i]);
    tft.print(buf, 4, 26);
    drawCrosshair(corners[i][0], corners[i][1], COLOR_YELLOW);

    Serial.print(F("Toque no canto ")); Serial.println(labels[i]);

    RawTouch t;
    bool got = waitForTouch(&t, 20000);
    if (got) {
      int16_t sx, sy;
      mapRawToScreen(t, &sx, &sy);
      int16_t dx = sx - corners[i][0], dy = sy - corners[i][1];
      bool near = (abs(dx) < 55 && abs(dy) < 55);
      if (!near) allOk = false;
      tft.setColor(near ? COLOR_GREEN : COLOR_RED);
      tft.fillCircle(sx, sy, 4);
      Serial.print(F("  Detectado em (")); Serial.print(sx); Serial.print(',');
      Serial.print(sy); Serial.print(F(") pressao~")); Serial.print(t.z);
      Serial.println(near ? F(" - OK") : F(" - fora da tolerancia"));
    } else {
      allOk = false;
      Serial.println(F("  Sem toque detectado (timeout/cancelado)."));
    }
    uiWaitMs(500);
  }
  return allOk;
}

static bool centerTouchTest() {
  int16_t cx = DISPLAY_WIDTH / 2, cy = DISPLAY_HEIGHT / 2;

  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 11: CENTRO");
  tft.setFont(SmallFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLACK);
  tft.print((char *)"Toque no centro da tela", 4, 26);
  drawCrosshair(cx, cy, COLOR_YELLOW);
  Serial.println(F("Toque no centro da tela."));

  RawTouch t;
  bool got = waitForTouch(&t, 20000);
  if (!got) {
    Serial.println(F("  Sem toque detectado (timeout/cancelado)."));
    return false;
  }
  int16_t sx, sy;
  mapRawToScreen(t, &sx, &sy);
  bool near = (abs(sx - cx) < 55 && abs(sy - cy) < 55);
  tft.setColor(near ? COLOR_GREEN : COLOR_RED);
  tft.fillCircle(sx, sy, 4);
  Serial.print(F("  Detectado em (")); Serial.print(sx); Serial.print(',');
  Serial.print(sy); Serial.print(F(") pressao~")); Serial.print(t.z);
  Serial.println(near ? F(" - OK") : F(" - fora da tolerancia"));
  uiWaitMs(500);
  return near;
}

// ----------------------------------------------------------------------------
// Modo de desenho livre com botao LIMPAR e botao VOLTAR
// ----------------------------------------------------------------------------
static void freeDrawCanvas() {
  const int16_t clearX = DISPLAY_WIDTH - 90, clearY = 26, clearW = 80, clearH = 26;
  const int16_t drawTop = 58, drawBottom = UI_BACK_BTN_Y - 6;

  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 11: TOQUE LIVRE");
  tft.setFont(SmallFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLACK);
  tft.print((char *)"Toque para desenhar", 4, 26);

  tft.setColor(COLOR_ORANGE);
  tft.fillRoundRect(clearX, clearY, clearX + clearW, clearY + clearH);
  tft.setColor(COLOR_BLACK);
  tft.setBackColor(COLOR_ORANGE);
  tft.print((char *)"LIMPAR", clearX + 10, clearY + 8);

  uiDrawBackButton();

  Serial.println(F("Modo de desenho livre. Toque na tela para desenhar."));
  Serial.println(F("Envie 'q' no Serial ou toque em VOLTAR para sair."));

  unsigned long lastStatus = 0;
  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'q' || c == 'Q') break;
    }
    RawTouch t;
    if (!readTouchFiltered(&t)) continue;

    int16_t sx, sy;
    mapRawToScreen(t, &sx, &sy);

    if (sx >= UI_BACK_BTN_X && sx <= UI_BACK_BTN_X + UI_BACK_BTN_W &&
        sy >= UI_BACK_BTN_Y && sy <= UI_BACK_BTN_Y + UI_BACK_BTN_H) {
      break;
    }
    if (sx >= clearX && sx <= clearX + clearW && sy >= clearY && sy <= clearY + clearH) {
      tft.setColor(COLOR_BLACK);
      tft.fillRect(0, drawTop, DISPLAY_WIDTH - 1, drawBottom);
      continue;
    }
    if (sy > drawTop && sy < drawBottom) {
      tft.setColor(COLOR_CYAN);
      tft.fillCircle(sx, sy, 2);
    }
    if (millis() - lastStatus > 200) {
      tft.setColor(COLOR_BLACK);
      tft.fillRect(0, 40, 160, 53);
      tft.setFont(SmallFont);
      tft.setColor(COLOR_WHITE);
      tft.setBackColor(COLOR_BLACK);
      char buf[32];
      snprintf(buf, sizeof(buf), "(%d,%d) P~%d", sx, sy, t.z);
      tft.print(buf, 4, 40);

      Serial.print(F("Toque em (")); Serial.print(sx); Serial.print(',');
      Serial.print(sy); Serial.print(F(") pressao~")); Serial.println(t.z);
      lastStatus = millis();
    }
  }
}

// ----------------------------------------------------------------------------
// Entrada do Teste 11
// ----------------------------------------------------------------------------
bool testTouch() {
  Serial.println(F("== TESTE 11: TOUCH =="));
  loadCalibration();

  RawTouch sanity = readTouchRaw();
  Serial.print(F("Leitura crua sem toque esperado: x="));
  Serial.print(sanity.x); Serial.print(F(" y=")); Serial.print(sanity.y);
  Serial.print(F(" z~")); Serial.println(sanity.z);

  bool calOk = runCalibration();
  if (!calOk) {
    tft.setColor(COLOR_BLACK);
    tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    uiDrawHeader("TESTE 11: TOUCH");
    tft.setFont(BigFont);
    tft.setColor(COLOR_RED);
    tft.setBackColor(COLOR_BLACK);
    tft.print((char *)"SEM RESPOSTA DO TOUCH", CENTER, 100);
    Serial.println(F("Resultado: FAIL (sem resposta na calibracao)"));
    Serial.println();
    return false;
  }

  bool cornersOk = cornerTouchTest();
  bool centerOk = centerTouchTest();
  freeDrawCanvas();

  bool pass = calOk && cornersOk && centerOk;
  Serial.println(pass ? F("Resultado: PASS")
                       : F("Resultado: FAIL (revise calibracao em docs/troubleshooting.md)"));
  Serial.println();
  return pass;
}

void touchInit() {
  pinMode(TOUCH_CLK, OUTPUT);
  pinMode(TOUCH_CS, OUTPUT);
  pinMode(TOUCH_DIN, OUTPUT);
  pinMode(TOUCH_DOUT, INPUT);
  digitalWrite(TOUCH_CS, HIGH);
  digitalWrite(TOUCH_CLK, LOW);
  loadCalibration();
}

bool touchPoll(int16_t *sx, int16_t *sy) {
  RawTouch t;
  if (!readTouchFiltered(&t)) return false;
  mapRawToScreen(t, sx, sy);
  return true;
}

#else // !TOUCH_HARDWARE_PRESENT

void touchInit() {}

bool touchPoll(int16_t *sx, int16_t *sy) {
  (void)sx; (void)sy;
  return false;
}

bool testTouch() {
  Serial.println(F("== TESTE 11: TOUCH =="));
  Serial.println(F("TOUCH NAO DISPONIVEL NESTE HARDWARE"));
  Serial.println(F("(TOUCH_HARDWARE_PRESENT esta desativado em config.h)"));
  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 11: TOUCH");
  tft.setFont(BigFont);
  tft.setColor(COLOR_RED);
  tft.setBackColor(COLOR_BLACK);
  tft.print((char *)"TOUCH NAO DISPONIVEL", CENTER, 100);
  tft.print((char *)"NESTE HARDWARE", CENTER, 130);
  Serial.println(F("Resultado: SKIPPED"));
  Serial.println();
  return false;
}

#endif // TOUCH_HARDWARE_PRESENT
