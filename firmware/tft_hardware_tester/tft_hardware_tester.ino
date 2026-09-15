// ============================================================================
// TFT HARDWARE TESTER
// ----------------------------------------------------------------------------
// Programa de diagnostico e teste de hardware para display TFT 320x240
// (controlador SSD1289) sobre TFT LCD Mega Shield V2.2 (ElecFreaks) +
// Arduino Mega 2560.
//
// Bibliotecas necessarias (ver README.md para instrucoes de instalacao -
// estas NAO estao no Gerenciador de Bibliotecas da Arduino IDE, precisam
// ser instaladas manualmente):
//   - UTFT      (Rinky-Dink Electronics / Henning Karlsen)
//   - URTouch   (nao usada para leitura - so os defines de referencia;
//                a leitura real do touch e feita por SPI manual em
//                test_touch.cpp, ver config.h para o motivo)
//
// Este arquivo cuida apenas de: setup/loop, menu Serial, menu grafico
// principal e o despacho para os modulos de teste (test_graphics.cpp,
// test_touch.cpp, test_performance.cpp).
// ============================================================================

#include <UTFT.h>

#include "config.h"
#include "state.h"
#include "ui.h"
#include "test_graphics.h"
#include "test_touch.h"
#include "test_performance.h"

UTFT tft(SSD1289, LCD_RS, LCD_WR, LCD_CS, LCD_RST);

// ----------------------------------------------------------------------------
// Tabela de testes (numero de menu -> nome -> funcao)
// ----------------------------------------------------------------------------
typedef bool (*TestFunc)();

struct TestEntry {
  uint8_t number;
  const char *name;
  TestFunc fn;
};

static const TestEntry TEST_TABLE[] = {
  {1,  "Inicializacao",       testInit},
  {2,  "Cores basicas",       testColors},
  {3,  "Gradiente de cores",  testGradient},
  {4,  "Pixels",              testPixels},
  {5,  "Linhas",              testLines},
  {6,  "Retangulos",          testRects},
  {7,  "Circulos",            testCircles},
  {8,  "Texto",               testText},
  {9,  "Orientacao",          testOrientation},
  {10, "Padrao diagnostico",  testDiagnosticPattern},
  {11, "Touch",               testTouch},
  {12, "Desempenho",          testPerformance},
};
#define TEST_TABLE_SIZE (sizeof(TEST_TABLE) / sizeof(TEST_TABLE[0]))

// ----------------------------------------------------------------------------
// Classificacao de resultado (trata Teste 11 como SKIPPED quando nao ha
// hardware de touch, em vez de FAIL - nao e uma falha, e um recurso ausente)
// ----------------------------------------------------------------------------
static TestResult classifyResult(uint8_t number, bool pass) {
#if !TOUCH_HARDWARE_PRESENT
  if (number == 11) return RESULT_SKIPPED;
#else
  (void)number;
#endif
  return pass ? RESULT_PASS : RESULT_FAIL;
}

static const char *resultToStr(TestResult r) {
  switch (r) {
    case RESULT_PASS: return "PASS";
    case RESULT_FAIL: return "FAIL";
    case RESULT_SKIPPED: return "SKIPPED";
    default: return "NAO EXECUTADO";
  }
}

// ----------------------------------------------------------------------------
// Menu grafico principal (grade de botoes grandes, touch-friendly)
// ----------------------------------------------------------------------------
#define MENU_COLS 4
#define MENU_ROWS 4
#define MENU_TOP 40
#define MENU_BOTTOM (DISPLAY_HEIGHT - 14)

static void drawMainMenu() {
  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TFT HARDWARE TESTER");

  tft.setFont(SmallFont);
  tft.setColor(COLOR_YELLOW);
  tft.setBackColor(COLOR_BLACK);
  char buf[40];
  snprintf(buf, sizeof(buf), "%dx%d  Ctrl:%s", DISPLAY_WIDTH, DISPLAY_HEIGHT, CONTROLLER_NAME);
  tft.print(buf, 4, 24);

  int16_t cellW = DISPLAY_WIDTH / MENU_COLS;
  int16_t cellH = (MENU_BOTTOM - MENU_TOP) / MENU_ROWS;

  for (uint8_t i = 0; i < TEST_TABLE_SIZE; i++) {
    uint8_t col = i % MENU_COLS, row = i / MENU_COLS;
    int16_t x = col * cellW + 3, y = MENU_TOP + row * cellH + 3;
    int16_t w = cellW - 6, h = cellH - 6;

    tft.setColor(COLOR_DARKGRAY);
    tft.fillRoundRect(x, y, x + w, y + h);
    tft.setColor(COLOR_WHITE);
    tft.drawRoundRect(x, y, x + w, y + h);

    tft.setFont(SmallFont);
    tft.setColor(COLOR_WHITE);
    tft.setBackColor(COLOR_DARKGRAY);
    char numBuf[4];
    snprintf(numBuf, sizeof(numBuf), "%d", TEST_TABLE[i].number);
    tft.print(numBuf, x + 4, y + 4);

    char abbrev[9];
    strncpy(abbrev, TEST_TABLE[i].name, 8);
    abbrev[8] = '\0';
    tft.print(abbrev, x + 4, y + h - 11);
  }

  // Slot extra da grade para "13 - Teste completo"
  {
    uint8_t i = TEST_TABLE_SIZE;
    uint8_t col = i % MENU_COLS, row = i / MENU_COLS;
    int16_t x = col * cellW + 3, y = MENU_TOP + row * cellH + 3;
    int16_t w = cellW - 6, h = cellH - 6;
    tft.setColor(COLOR_ORANGE);
    tft.fillRoundRect(x, y, x + w, y + h);
    tft.setColor(COLOR_WHITE);
    tft.drawRoundRect(x, y, x + w, y + h);
    tft.setColor(COLOR_BLACK);
    tft.setBackColor(COLOR_ORANGE);
    tft.print((char *)"13", x + 4, y + 4);
    tft.print((char *)"TUDO", x + 4, y + h - 11);
  }

  tft.setFont(SmallFont);
  tft.setColor(COLOR_CYAN);
  tft.setBackColor(COLOR_BLACK);
  snprintf(buf, sizeof(buf), "Testes executados: %u", g_testsExecuted);
  tft.print(buf, 4, DISPLAY_HEIGHT - 12);
}

// Retorna o numero do teste tocado no menu grafico (1-13), ou 0 se o toque
// nao caiu em nenhum botao.
static uint8_t mainMenuHitTest(int16_t sx, int16_t sy) {
  if (sy < MENU_TOP || sy >= MENU_BOTTOM) return 0;
  int16_t cellW = DISPLAY_WIDTH / MENU_COLS;
  int16_t cellH = (MENU_BOTTOM - MENU_TOP) / MENU_ROWS;
  uint8_t col = sx / cellW;
  uint8_t row = (sy - MENU_TOP) / cellH;
  uint8_t index = row * MENU_COLS + col;
  if (index < TEST_TABLE_SIZE) return TEST_TABLE[index].number;
  if (index == TEST_TABLE_SIZE) return 13; // slot "TUDO"
  return 0;
}

// ----------------------------------------------------------------------------
// Espera o usuario confirmar a leitura do resultado (toque no botao VOLTAR
// ou qualquer tecla no Serial). Tem um timeout generoso para nao travar o
// programa caso ninguem interaja.
// ----------------------------------------------------------------------------
static void waitForBackOrEnter(unsigned long timeoutMs = 30000) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    if (Serial.available()) {
      while (Serial.available()) Serial.read();
      return;
    }
    int16_t sx, sy;
    if (touchPoll(&sx, &sy)) {
      if (sx >= UI_BACK_BTN_X && sx <= UI_BACK_BTN_X + UI_BACK_BTN_W &&
          sy >= UI_BACK_BTN_Y && sy <= UI_BACK_BTN_Y + UI_BACK_BTN_H) {
        return;
      }
    }
  }
}

// Depois de um teste que usou touch (ex.: toque em VOLTAR), o dedo pode
// ainda estar encostado quando o menu principal e redesenhado. Descarta
// leituras por uma janela curta apos redesenhar o menu.
static void settleMenuTouch() {
  unsigned long start = millis();
  while (millis() - start < 400) {
    int16_t sx, sy;
    touchPoll(&sx, &sy);
  }
}

// ----------------------------------------------------------------------------
// Executa um teste avulso escolhido pelo menu (Serial ou touch)
// ----------------------------------------------------------------------------
static void runTest(uint8_t number) {
  for (uint8_t i = 0; i < TEST_TABLE_SIZE; i++) {
    if (TEST_TABLE[i].number != number) continue;

    unsigned long t0 = millis();
    bool pass = TEST_TABLE[i].fn();
    unsigned long dur = millis() - t0;
    TestResult result = classifyResult(number, pass);
    stateRecordResult(number, TEST_TABLE[i].name, result, dur);

    uiDrawBackButton();
    tft.setFont(SmallFont);
    tft.setColor(result == RESULT_PASS ? COLOR_GREEN
                 : result == RESULT_SKIPPED ? COLOR_YELLOW : COLOR_RED);
    tft.setBackColor(COLOR_BLACK);
    tft.print((char *)resultToStr(result), 150, UI_BACK_BTN_Y + 10);

    Serial.print(F("Duracao do teste: "));
    Serial.print(dur);
    Serial.println(F(" ms"));
    Serial.println(F("(toque em VOLTAR ou pressione ENTER no Serial para continuar)"));

    waitForBackOrEnter();
    return;
  }
  Serial.println(F("Opcao invalida."));
}

// ----------------------------------------------------------------------------
// Teste completo automatico (opcao 13)
// ----------------------------------------------------------------------------
static void runAllTests() {
  Serial.println(F("===== INICIANDO TESTE COMPLETO ====="));
  for (uint8_t i = 0; i < TEST_TABLE_SIZE; i++) {
    tft.setColor(COLOR_BLACK);
    tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    uiDrawHeader("TESTE COMPLETO");
    tft.setFont(BigFont);
    tft.setColor(COLOR_YELLOW);
    tft.setBackColor(COLOR_BLACK);
    tft.print((char *)TEST_TABLE[i].name, CENTER, 100);
    char progress[16];
    snprintf(progress, sizeof(progress), "%u / %u", (unsigned)(i + 1), (unsigned)TEST_TABLE_SIZE);
    tft.setColor(COLOR_WHITE);
    tft.print(progress, CENTER, 140);
    Serial.print(F("Executando teste "));
    Serial.print(TEST_TABLE[i].number);
    Serial.print(F(" - "));
    Serial.println(TEST_TABLE[i].name);
    uiWaitMs(800);

    unsigned long t0 = millis();
    bool pass = TEST_TABLE[i].fn();
    unsigned long dur = millis() - t0;
    TestResult result = classifyResult(TEST_TABLE[i].number, pass);
    stateRecordResult(TEST_TABLE[i].number, TEST_TABLE[i].name, result, dur);

    if (result == RESULT_SKIPPED) {
      tft.setColor(COLOR_YELLOW);
      tft.fillRect(40, 90, 280, 150);
      tft.setColor(COLOR_WHITE);
      tft.drawRect(40, 90, 280, 150);
      tft.setFont(BigFont);
      tft.setColor(COLOR_BLACK);
      tft.setBackColor(COLOR_YELLOW);
      tft.print((char *)"SKIPPED", CENTER, 112);
    } else {
      uiShowResultBanner(result == RESULT_PASS);
    }
    uiWaitMs(1500);
  }

  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
  uiDrawHeader("TESTE COMPLETO");
  tft.setFont(BigFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLACK);
  tft.print((char *)"TESTE COMPLETO", CENTER, 90);
  tft.print((char *)"FINALIZADO", CENTER, 115);

  Serial.println();
  Serial.println(F("===== RESUMO DO TESTE COMPLETO ====="));
  uint8_t passCount = 0, failCount = 0, skipCount = 0;
  for (uint8_t i = 0; i < TEST_TABLE_SIZE; i++) {
    uint8_t n = TEST_TABLE[i].number;
    TestRecord &r = g_results[n];
    Serial.print(n);
    Serial.print(F(" - "));
    Serial.print(r.name);
    Serial.print(F(": "));
    Serial.print(resultToStr(r.result));
    Serial.print(F(" ("));
    Serial.print(r.durationMs);
    Serial.println(F(" ms)"));
    if (r.result == RESULT_PASS) passCount++;
    else if (r.result == RESULT_SKIPPED) skipCount++;
    else failCount++;
  }
  Serial.print(F("Total PASS: ")); Serial.println(passCount);
  Serial.print(F("Total FAIL: ")); Serial.println(failCount);
  Serial.print(F("Total SKIPPED: ")); Serial.println(skipCount);
  Serial.println(F("TESTE COMPLETO FINALIZADO"));
  Serial.println();

  uiWaitMs(1500);
}

// ----------------------------------------------------------------------------
// Menu Serial
// ----------------------------------------------------------------------------
static void printSerialMenu() {
  Serial.println(F("================================"));
  Serial.println(F("TFT HARDWARE TESTER"));
  Serial.println(F("1 - Inicializacao"));
  Serial.println(F("2 - Cores"));
  Serial.println(F("3 - Gradiente"));
  Serial.println(F("4 - Pixels"));
  Serial.println(F("5 - Linhas"));
  Serial.println(F("6 - Retangulos"));
  Serial.println(F("7 - Circulos"));
  Serial.println(F("8 - Texto"));
  Serial.println(F("9 - Orientacao"));
  Serial.println(F("10 - Padrao diagnostico"));
  Serial.println(F("11 - Touch"));
  Serial.println(F("12 - Desempenho"));
  Serial.println(F("13 - Teste completo"));
  Serial.println(F("0 - Voltar"));
  Serial.println(F("================================"));
  Serial.print(F("Escolha uma opcao: "));
}

static void handleMenuChoice(int choice) {
  Serial.println(choice);
  if (choice == 0) {
    drawMainMenu();
  } else if (choice == 13) {
    runAllTests();
    drawMainMenu();
    settleMenuTouch();
  } else if (choice >= 1 && choice <= 12) {
    runTest((uint8_t)choice);
    drawMainMenu();
    settleMenuTouch();
  } else {
    Serial.println(F("Opcao invalida."));
  }
  printSerialMenu();
}

// Leitor de linha nao-bloqueante para o Serial Monitor
static char serialBuf[8];
static uint8_t serialBufLen = 0;

static void pollSerialMenu() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialBufLen > 0) {
        serialBuf[serialBufLen] = '\0';
        int choice = atoi(serialBuf);
        serialBufLen = 0;
        handleMenuChoice(choice);
      }
    } else if (isDigit(c) && serialBufLen < sizeof(serialBuf) - 1) {
      serialBuf[serialBufLen++] = c;
    }
  }
}

// ----------------------------------------------------------------------------
// setup / loop
// ----------------------------------------------------------------------------
void setup() {
  Serial.begin(SERIAL_BAUD);
  unsigned long serialWaitStart = millis();
  while (!Serial && millis() - serialWaitStart < 3000) {
    // aguarda o Serial Monitor conectar
  }

  touchInit();

  // Inicializacao "silenciosa" do display para que o menu grafico apareca
  // imediatamente ao ligar (nao conta como execucao do Teste 1 - isso so
  // acontece quando o usuario escolhe a opcao 1 explicitamente).
  testInit();

  drawMainMenu();

  Serial.println();
  Serial.println(F("TFT HARDWARE TESTER pronto."));
  Serial.print(F("Controlador: "));
  Serial.println(CONTROLLER_NAME);
  printSerialMenu();
}

void loop() {
  pollSerialMenu();

  int16_t sx, sy;
  if (touchPoll(&sx, &sy)) {
    uint8_t number = mainMenuHitTest(sx, sy);
    if (number != 0) {
      handleMenuChoice(number);
    }
  }
}
