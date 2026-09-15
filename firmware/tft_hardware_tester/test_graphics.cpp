#include "test_graphics.h"

// ----------------------------------------------------------------------------
// Teste 1 - Inicializacao (inclui diagnostico basico de comunicacao: a UTFT
// nao oferece leitura do ID do controlador - ver config.h/docs/hardware.md
// sobre como o SSD1289 foi confirmado em bancada. Aqui o teste garante que
// a sequencia de init roda sem travar e que a resolucao bate com o
// esperado).
// ----------------------------------------------------------------------------
bool testInit() {
  Serial.println(F("== TESTE 1: INICIALIZACAO =="));

  tft.InitLCD(LANDSCAPE);
  tft.clrScr();

  int w = tft.getDisplayXSize();
  int h = tft.getDisplayYSize();
  bool resolutionOk = (w == DISPLAY_WIDTH && h == DISPLAY_HEIGHT);

  uiDrawHeader("TESTE 1: INIT");
  tft.setFont(BigFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLACK);
  tft.print((char *)"Resolucao:", 6, 34);
  char buf[16];
  snprintf(buf, sizeof(buf), "%d x %d", w, h);
  tft.print(buf, 6, 58);

  tft.print((char *)"Controlador:", 6, 90);
  tft.print((char *)CONTROLLER_NAME, 6, 114);

  Serial.print(F("Resolucao detectada: "));
  Serial.print(w);
  Serial.print('x');
  Serial.println(h);
  Serial.print(F("Controlador (confirmado em bancada): "));
  Serial.println(CONTROLLER_NAME);

  if (!resolutionOk) {
    Serial.println(F("AVISO: resolucao detectada != 320x240 esperado."));
    tft.setColor(COLOR_RED);
    tft.print((char *)"RESOLUCAO INESPERADA", 6, 140);
  }

  Serial.println(resolutionOk ? F("Resultado: PASS") : F("Resultado: FAIL"));
  Serial.println();
  return resolutionOk;
}

// ----------------------------------------------------------------------------
// Teste 2 - Cores basicas
// ----------------------------------------------------------------------------
bool testColors() {
  Serial.println(F("== TESTE 2: CORES BASICAS =="));
  struct { const char *name; uint16_t color; uint16_t textColor; } colors[] = {
    {"PRETO",   COLOR_BLACK,   COLOR_WHITE},
    {"BRANCO",  COLOR_WHITE,   COLOR_BLACK},
    {"VERMELHO",COLOR_RED,     COLOR_WHITE},
    {"VERDE",   COLOR_GREEN,   COLOR_BLACK},
    {"AZUL",    COLOR_BLUE,    COLOR_WHITE},
    {"AMARELO", COLOR_YELLOW,  COLOR_BLACK},
    {"CIANO",   COLOR_CYAN,    COLOR_BLACK},
    {"MAGENTA", COLOR_MAGENTA, COLOR_WHITE},
  };
  for (uint8_t i = 0; i < 8; i++) {
    tft.setColor(colors[i].color);
    tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    tft.setFont(BigFont);
    tft.setColor(colors[i].textColor);
    tft.setBackColor(colors[i].color);
    tft.print((char *)colors[i].name, CENTER, 110);

    Serial.print(F("  Cor exibida: "));
    Serial.println(colors[i].name);
    uiWaitMs(600);
  }
  Serial.println(F("Resultado: PASS (teste visual - confira se as cores batem com os nomes)"));
  Serial.println();
  return true;
}

// ----------------------------------------------------------------------------
// Teste 3 - Gradientes horizontais e verticais
// ----------------------------------------------------------------------------
static void gradientHorizontal(uint8_t channel) {
  for (int16_t x = 0; x < DISPLAY_WIDTH; x++) {
    uint8_t level5 = map(x, 0, DISPLAY_WIDTH - 1, 0, 31);
    uint8_t level6 = map(x, 0, DISPLAY_WIDTH - 1, 0, 63);
    uint16_t color;
    switch (channel) {
      case 0: color = level5 << 11; break;
      case 1: color = level6 << 5; break;
      case 2: color = level5; break;
      default: color = (level5 << 11) | (level6 << 5) | level5; break;
    }
    tft.setColor(color);
    tft.drawLine(x, 23, x, DISPLAY_HEIGHT - 1);
  }
}

static void gradientVertical(uint8_t channel) {
  for (int16_t y = 23; y < DISPLAY_HEIGHT; y++) {
    uint8_t level5 = map(y, 23, DISPLAY_HEIGHT - 1, 0, 31);
    uint8_t level6 = map(y, 23, DISPLAY_HEIGHT - 1, 0, 63);
    uint16_t color;
    switch (channel) {
      case 0: color = level5 << 11; break;
      case 1: color = level6 << 5; break;
      case 2: color = level5; break;
      default: color = (level5 << 11) | (level6 << 5) | level5; break;
    }
    tft.setColor(color);
    tft.drawLine(0, y, DISPLAY_WIDTH - 1, y);
  }
}

bool testGradient() {
  Serial.println(F("== TESTE 3: GRADIENTE DE CORES =="));
  const char *names[4] = {"VERMELHO", "VERDE", "AZUL", "RGB"};
  for (uint8_t ch = 0; ch < 4; ch++) {
    uiDrawHeader("TESTE 3: GRAD H");
    gradientHorizontal(ch);
    tft.setFont(SmallFont);
    tft.setColor(COLOR_WHITE);
    tft.setBackColor(COLOR_BLACK);
    char buf[24];
    snprintf(buf, sizeof(buf), "Horizontal: %s", names[ch]);
    tft.print(buf, 4, 24);
    Serial.print(F("  Gradiente horizontal ")); Serial.println(names[ch]);
    uiWaitMs(700);

    uiDrawHeader("TESTE 3: GRAD V");
    gradientVertical(ch);
    tft.setFont(SmallFont);
    tft.setColor(COLOR_WHITE);
    tft.setBackColor(COLOR_BLACK);
    snprintf(buf, sizeof(buf), "Vertical: %s", names[ch]);
    tft.print(buf, 4, 24);
    Serial.print(F("  Gradiente vertical ")); Serial.println(names[ch]);
    uiWaitMs(700);
  }
  Serial.println(F("Resultado: PASS (teste visual - confira faixas continuas sem bandas/falhas)"));
  Serial.println();
  return true;
}

// ----------------------------------------------------------------------------
// Teste 4 - Pixels (cantos, centro, bordas, padrao de pontos). A UTFT nao
// suporta leitura de GRAM, entao este teste e puramente visual.
// ----------------------------------------------------------------------------
bool testPixels() {
  Serial.println(F("== TESTE 4: PIXELS =="));
  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 4: PIXELS");

  int16_t pts[][2] = {
    {0, 23}, {DISPLAY_WIDTH - 1, 23}, {0, DISPLAY_HEIGHT - 1}, {DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1},
    {DISPLAY_WIDTH / 2, (DISPLAY_HEIGHT + 23) / 2},
    {DISPLAY_WIDTH / 2, 23}, {DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 1},
    {0, (DISPLAY_HEIGHT + 23) / 2}, {DISPLAY_WIDTH - 1, (DISPLAY_HEIGHT + 23) / 2},
  };
  const uint8_t nPts = sizeof(pts) / sizeof(pts[0]);
  tft.setColor(COLOR_RED);
  for (uint8_t i = 0; i < nPts; i++) {
    tft.drawPixel(pts[i][0], pts[i][1]);
  }

  tft.setColor(COLOR_GREEN);
  for (int16_t y = 30; y < DISPLAY_HEIGHT; y += 10) {
    for (int16_t x = 10; x < DISPLAY_WIDTH; x += 10) {
      tft.drawPixel(x, y);
    }
  }

  tft.setFont(SmallFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLACK);
  tft.print((char *)"Inspecione: cantos/centro em vermelho,", 4, 24);
  tft.print((char *)"grade em verde. Pixels apagados = defeito.", 4, 34);

  Serial.print(F("Pontos de referencia desenhados: "));
  Serial.println(nPts);
  Serial.println(F("IMPORTANTE: confira visualmente se ha pixels apagados/presos na grade verde."));
  Serial.println(F("Resultado: PASS (pendente confirmacao visual - sem leitura de GRAM nesta biblioteca)"));
  Serial.println();
  return true;
}

// ----------------------------------------------------------------------------
// Teste 5 - Linhas
// ----------------------------------------------------------------------------
bool testLines() {
  Serial.println(F("== TESTE 5: LINHAS =="));
  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 5: LINHAS");

  int16_t top = 26, bottom = DISPLAY_HEIGHT - 1;

  tft.setColor(COLOR_CYAN);
  for (int16_t y = top; y < bottom; y += 20) tft.drawLine(0, y, DISPLAY_WIDTH - 1, y);
  tft.setColor(COLOR_YELLOW);
  for (int16_t x = 0; x < DISPLAY_WIDTH; x += 20) tft.drawLine(x, top, x, bottom);
  tft.setColor(COLOR_RED);
  tft.drawLine(0, top, DISPLAY_WIDTH - 1, bottom);
  tft.drawLine(DISPLAY_WIDTH - 1, top, 0, bottom);
  tft.setColor(COLOR_WHITE);
  tft.drawRect(0, top, DISPLAY_WIDTH - 1, bottom);

  Serial.println(F("Linhas horizontais, verticais, diagonais e moldura desenhadas."));
  Serial.println(F("Resultado: PASS (teste visual - confira linhas retas, sem quebras)"));
  Serial.println();
  return true;
}

// ----------------------------------------------------------------------------
// Teste 6 - Retangulos
// ----------------------------------------------------------------------------
bool testRects() {
  Serial.println(F("== TESTE 6: RETANGULOS =="));
  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 6: RETANGULOS");

  tft.setColor(COLOR_WHITE);
  tft.drawRect(10, 30, 100, 90);                 // vazio

  tft.setColor(COLOR_RED);
  tft.fillRect(110, 30, 200, 90);                // preenchido

  tft.setColor(COLOR_GREEN);
  tft.drawRect(210, 30, 260, 80);
  tft.setColor(COLOR_YELLOW);
  tft.drawRect(214, 34, 256, 76);
  tft.setColor(COLOR_CYAN);
  tft.drawRect(218, 38, 252, 72);                // molduras concentricas

  tft.setColor(COLOR_BLUE);
  tft.fillRect(10, 110, 100, 170);
  tft.setColor(COLOR_WHITE);
  tft.drawRect(10, 110, 100, 170);               // preenchido com borda

  Serial.println(F("Retangulos vazio, preenchido, quadrado com molduras e com borda desenhados."));
  Serial.println(F("Resultado: PASS (teste visual - confira proporcoes e preenchimento uniforme)"));
  Serial.println();
  return true;
}

// ----------------------------------------------------------------------------
// Teste 7 - Circulos
// ----------------------------------------------------------------------------
bool testCircles() {
  Serial.println(F("== TESTE 7: CIRCULOS =="));
  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 7: CIRCULOS");

  tft.setColor(COLOR_MAGENTA);
  tft.fillCircle(70, 90, 35);      // preenchido
  tft.setColor(COLOR_GREEN);
  tft.drawCircle(200, 90, 35);     // vazio

  int16_t cx = 260, cy = 170;
  for (int16_t r = 10; r <= 50; r += 10) {
    tft.setColor((r / 10) % 2 == 0 ? COLOR_YELLOW : COLOR_CYAN);
    tft.drawCircle(cx, cy, r);
  }

  Serial.println(F("Circulo preenchido, vazio e concentricos desenhados."));
  Serial.println(F("Resultado: PASS (teste visual - confira bordas lisas e preenchimento uniforme)"));
  Serial.println();
  return true;
}

// ----------------------------------------------------------------------------
// Teste 8 - Texto
// ----------------------------------------------------------------------------
bool testText() {
  Serial.println(F("== TESTE 8: TEXTO =="));
  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE 8: TEXTO");

  int16_t y = 26;
  tft.setFont(SmallFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLACK);
  tft.print((char *)"Texto pequeno 0123456789", 4, y);
  y += 16;

  tft.setFont(BigFont);
  tft.setColor(COLOR_WHITE);
  tft.print((char *)"Texto medio ABC xyz", 4, y);
  y += 24;

  tft.setColor(COLOR_YELLOW);
  tft.print((char *)"GRANDE", 4, y);
  y += 32;

  tft.setColor(COLOR_CYAN);
  tft.print((char *)"!@#$%&*()-+=/?", 4, y);
  y += 26;

  tft.setColor(COLOR_BLACK);
  tft.setBackColor(COLOR_ORANGE);
  tft.setColor(COLOR_ORANGE);
  tft.fillRect(4, y, 200, y + 24);
  tft.setColor(COLOR_BLACK);
  tft.print((char *)"Fundo colorido", 8, y + 4);
  y += 30;

  tft.setFont(SmallFont);
  tft.setColor(COLOR_GREEN);
  tft.setBackColor(COLOR_BLACK);
  tft.print((char *)"TFT 2.4\"", 4, y);
  tft.print((char *)"320 x 240", 4, y + 10);
  tft.print((char *)"Arduino Mega 2560", 4, y + 20);

  Serial.println(F("Texto em tamanhos pequeno/medio/grande, numeros, simbolos, cores e fundo colorido exibidos."));
  Serial.println(F("Resultado: PASS (teste visual - confira legibilidade e ausencia de cortes)"));
  Serial.println();
  return true;
}

// ----------------------------------------------------------------------------
// Teste 9 - Orientacao
// ----------------------------------------------------------------------------
// A biblioteca UTFT so suporta 2 orientacoes para este controlador
// (PORTRAIT e LANDSCAPE) - nao ha um modo nativo de "invertido" (180
// graus) exposto pela API. Por isso este teste cobre as 2 orientacoes
// realmente disponiveis, em vez das 4 pedidas originalmente. Isso esta
// documentado em README.md ("Limitacoes conhecidas").
// ----------------------------------------------------------------------------
static void drawOrientationScreen(uint8_t orient, const char *label) {
  tft.InitLCD(orient);
  tft.clrScr();
  int w = tft.getDisplayXSize();
  int h = tft.getDisplayYSize();

  tft.setColor(COLOR_DARKGRAY);
  for (int16_t x = 0; x < w; x += 20) tft.drawLine(x, 0, x, h - 1);
  for (int16_t y = 0; y < h; y += 20) tft.drawLine(0, y, w - 1, y);

  tft.setColor(COLOR_RED);
  tft.drawLine(0, h / 2, w - 1, h / 2);
  tft.setColor(COLOR_GREEN);
  tft.drawLine(w / 2, 0, w / 2, h - 1);

  tft.setFont(BigFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLACK);
  tft.print((char *)label, 4, 4);

  tft.setFont(SmallFont);
  char buf[16];
  snprintf(buf, sizeof(buf), "%dx%d", w, h);
  tft.print(buf, 4, h - 12);
}

bool testOrientation() {
  Serial.println(F("== TESTE 9: ORIENTACAO =="));
  bool allOk = true;

  drawOrientationScreen(PORTRAIT, (char *)"PORTRAIT");
  bool p1 = (tft.getDisplayXSize() == DISPLAY_HEIGHT && tft.getDisplayYSize() == DISPLAY_WIDTH);
  Serial.print(F("  Portrait -> ")); Serial.print(tft.getDisplayXSize()); Serial.print('x');
  Serial.print(tft.getDisplayYSize()); Serial.println(p1 ? F(" OK") : F(" INESPERADO"));
  if (!p1) allOk = false;
  uiWaitMs(1500);

  drawOrientationScreen(LANDSCAPE, (char *)"LANDSCAPE");
  bool p2 = (tft.getDisplayXSize() == DISPLAY_WIDTH && tft.getDisplayYSize() == DISPLAY_HEIGHT);
  Serial.print(F("  Landscape -> ")); Serial.print(tft.getDisplayXSize()); Serial.print('x');
  Serial.print(tft.getDisplayYSize()); Serial.println(p2 ? F(" OK") : F(" INESPERADO"));
  if (!p2) allOk = false;
  uiWaitMs(1500);

  // Volta para o modo padrao usado no resto do firmware
  tft.InitLCD(LANDSCAPE);
  tft.clrScr();

  Serial.println(F("Nota: esta biblioteca/controlador so suporta 2 orientacoes (nao ha modo invertido nativo)."));
  Serial.println(allOk ? F("Resultado: PASS") : F("Resultado: FAIL"));
  Serial.println();
  return allOk;
}

// ----------------------------------------------------------------------------
// Teste 10 - Padrao de diagnostico completo
// ----------------------------------------------------------------------------
bool testDiagnosticPattern() {
  Serial.println(F("== TESTE 10: PADRAO DE DIAGNOSTICO =="));
  tft.InitLCD(LANDSCAPE);
  tft.clrScr();
  int w = tft.getDisplayXSize();
  int h = tft.getDisplayYSize();

  tft.setColor(COLOR_DARKGRAY);
  for (int16_t x = 0; x < w; x += 20) tft.drawLine(x, 0, x, h - 1);
  for (int16_t y = 0; y < h; y += 20) tft.drawLine(0, y, w - 1, y);

  tft.setColor(COLOR_WHITE);
  tft.drawRect(0, 0, w - 1, h - 1);
  tft.drawRect(1, 1, w - 2, h - 2);

  tft.setColor(COLOR_RED);
  tft.drawCircle(15, 15, 12);
  tft.setColor(COLOR_GREEN);
  tft.drawCircle(w - 16, 15, 12);
  tft.setColor(COLOR_BLUE);
  tft.drawCircle(15, h - 16, 12);
  tft.setColor(COLOR_YELLOW);
  tft.drawCircle(w - 16, h - 16, 12);

  int16_t cx = w / 2, cy = h / 2;
  tft.setColor(COLOR_CYAN);
  tft.drawLine(cx - 15, cy, cx + 15, cy);
  tft.drawLine(cx, cy - 15, cx, cy + 15);

  tft.setFont(SmallFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLACK);
  char buf[16];
  snprintf(buf, sizeof(buf), "(%d,%d)", cx, cy);
  tft.print(buf, cx - 24, cy + 4);

  tft.setColor(COLOR_RED);    tft.print((char *)"SUP-ESQ", 4, 4);
  tft.setColor(COLOR_GREEN);  tft.print((char *)"SUP-DIR", w - 56, 4);
  tft.setColor(COLOR_BLUE);   tft.print((char *)"INF-ESQ", 4, h - 12);
  tft.setColor(COLOR_YELLOW); tft.print((char *)"INF-DIR", w - 56, h - 12);

  uint16_t refColors[8] = {COLOR_BLACK, COLOR_WHITE, COLOR_RED, COLOR_GREEN,
                            COLOR_BLUE, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA};
  int16_t barW = w / 8;
  for (uint8_t i = 0; i < 8; i++) {
    tft.setColor(refColors[i]);
    tft.fillRect(i * barW, h / 2 + 30, i * barW + barW - 1, h / 2 + 49);
  }
  tft.setColor(COLOR_WHITE);
  tft.drawRect(0, h / 2 + 30, barW * 8 - 1, h / 2 + 49);

  Serial.println(F("Padrao completo desenhado: grade, moldura, circulos nos cantos,"));
  Serial.println(F("cruz central, coordenadas, texto nos 4 cantos e barra de cores."));
  Serial.println(F("Resultado: PASS (use esta tela para inspecao visual final)"));
  Serial.println();
  return true;
}
