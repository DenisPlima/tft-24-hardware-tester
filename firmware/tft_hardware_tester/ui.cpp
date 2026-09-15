#include "ui.h"

const int16_t UI_BACK_BTN_X = 10;
const int16_t UI_BACK_BTN_Y = DISPLAY_HEIGHT - 40;
const int16_t UI_BACK_BTN_W = 130;
const int16_t UI_BACK_BTN_H = 32;

void uiDrawHeader(const char *title) {
  tft.setColor(COLOR_BLUE);
  tft.fillRect(0, 0, DISPLAY_WIDTH - 1, 21);
  tft.setFont(BigFont);
  tft.setColor(COLOR_WHITE);
  tft.setBackColor(COLOR_BLUE);
  tft.print((char *)title, 4, 3);
}

void uiDrawBackButton() {
  tft.setColor(COLOR_DARKGRAY);
  tft.fillRoundRect(UI_BACK_BTN_X, UI_BACK_BTN_Y, UI_BACK_BTN_X + UI_BACK_BTN_W, UI_BACK_BTN_Y + UI_BACK_BTN_H);
  tft.setColor(COLOR_WHITE);
  tft.drawRoundRect(UI_BACK_BTN_X, UI_BACK_BTN_Y, UI_BACK_BTN_X + UI_BACK_BTN_W, UI_BACK_BTN_Y + UI_BACK_BTN_H);
  tft.setFont(BigFont);
  tft.setBackColor(COLOR_DARKGRAY);
  tft.print((char *)"VOLTAR", UI_BACK_BTN_X + 14, UI_BACK_BTN_Y + 9);
}

void uiClearContent() {
  tft.setColor(COLOR_BLACK);
  tft.fillRect(0, 23, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
}

void uiShowResultBanner(bool pass) {
  uint16_t bg = pass ? COLOR_GREEN : COLOR_RED;
  const char *msg = pass ? "PASS" : "FAIL";
  tft.setColor(bg);
  tft.fillRect(40, 90, 280, 150);
  tft.setColor(COLOR_WHITE);
  tft.drawRect(40, 90, 280, 150);
  tft.setFont(BigFont);
  tft.setBackColor(bg);
  tft.setColor(COLOR_BLACK);
  tft.print((char *)msg, CENTER, 112);
}

void uiWaitMs(unsigned long ms) {
  delay(ms);
}
