// ============================================================================
// test_graphics.h - Testes 1 a 10 (inicializacao ate padrao de diagnostico)
// ============================================================================
#ifndef TEST_GRAPHICS_H
#define TEST_GRAPHICS_H

#include <UTFT.h>
#include "config.h"
#include "state.h"
#include "ui.h"

bool testInit();               // Teste 1
bool testColors();             // Teste 2
bool testGradient();           // Teste 3
bool testPixels();             // Teste 4
bool testLines();              // Teste 5
bool testRects();              // Teste 6
bool testCircles();            // Teste 7
bool testText();                // Teste 8
bool testOrientation();        // Teste 9
bool testDiagnosticPattern();  // Teste 10

#endif // TEST_GRAPHICS_H
