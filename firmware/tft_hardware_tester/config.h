// ============================================================================
// config.h - Configuracao central do TFT Hardware Tester
// Hardware alvo: Arduino Mega 2560 + TFT LCD Mega Shield V2.2 (ElecFreaks) +
// display LM32US01 (controlador SSD1289), 320x240.
// ============================================================================
//
// HISTORICO DA DESCOBERTA (ver docs/hardware.md para a analise completa):
// A primeira versao deste firmware usava a biblioteca MCUFRIEND_kbv, que
// pressupoe um barramento de 8 bits. Testado no hardware real, isso NUNCA
// produziu imagem, em 5 variantes de pinagem diferentes. A causa raiz: este
// shield especifico (ElecFreaks TFT LCD Mega Shield V2.2) usa o display em
// modo PARALELO DE 16 BITS DE VERDADE, com o barramento de dados ocupando
// PORTA (D22-D29, byte alto) + PORTC (D30-D37, byte baixo) do Mega2560 -
// pinos exclusivos do Mega, fora do header compativel com Uno. A biblioteca
// correta para esse modo e a UTFT (Rinky-Dink Electronics / Henning Karlsen),
// confirmada por: (1) esquematico oficial do shield encontrado no forum
// Arduino, (2) um exemplo de codigo real e funcional no GitHub
// (milstrike/TFT-LCD-Arduino) usando exatamente este shield, e (3) TESTADO
// NO HARDWARE REAL DESTE PROJETO com sucesso (tela mostrou as 4 cores
// esperadas: vermelho, verde, azul, amarelo).
//
// Pinagem CONFIRMADA (esquematico + exemplo real + teste em bancada):
//   Controlador: SSD1289
//   RS   -> D38        WR   -> D39        CS   -> D40       RESET -> D41
//   Barramento de dados: gerenciado internamente pela biblioteca UTFT
//   (PORTA = byte alto / D22-D29, PORTC = byte baixo / D30-D37) - nao
//   precisa ser configurado neste arquivo, a UTFT faz isso sozinha ao
//   detectar __AVR_ATmega2560__.
//
// Touch resistivo (controlador XPT2046), pinagem tambem confirmada por
// esquematico + exemplo real, MAS com uma correcao encontrada em bancada:
// o pino de IRQ (D2) NAO responde neste shield (testado e confirmado sem
// nenhuma mudanca de estado ao tocar). Por isso o firmware NAO usa IRQ para
// detectar toque - ele le o canal X/Y por SPI manual continuamente e
// descarta leituras invalidas (extremos de escala = sem contato), com
// filtro e debounce proprios. Isso foi validado em bancada: os valores
// brutos de X/Y mudam de forma consistente ao tocar a tela.
//   T_CLK -> D6   T_CS -> D5   T_DIN -> D4   T_DOUT -> D3   (T_IRQ -> D2, nao usado)
// ============================================================================

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ----------------------------------------------------------------------------
// Resolucao do display
// ----------------------------------------------------------------------------
#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  240

// ----------------------------------------------------------------------------
// Pinos do display (passados ao construtor UTFT em tft_hardware_tester.ino)
// ----------------------------------------------------------------------------
#define LCD_RS   38
#define LCD_WR   39
#define LCD_CS   40
#define LCD_RST  41

// ----------------------------------------------------------------------------
// Disponibilidade e pinagem do touch resistivo
// ----------------------------------------------------------------------------
// Confirmado fisicamente pelo usuario (filme resistivo visivel sobre o
// vidro) e depois confirmado eletricamente em bancada (valores de X/Y
// respondem ao toque via leitura SPI manual). Para desativar completamente
// o teste de touch, comente a linha abaixo.
#define TOUCH_HARDWARE_PRESENT 1

#define TOUCH_CLK   6
#define TOUCH_CS    5
#define TOUCH_DIN   4
#define TOUCH_DOUT  3
// T_IRQ (D2) existe no esquematico mas nao respondeu em bancada neste
// shield - mantido aqui apenas para referencia/depuracao futura, nao e
// usado pela logica de leitura do touch.
#define TOUCH_IRQ   2

// Faixas brutas do ADC do XPT2046 (12 bits, 0-4095) consideradas "sem
// contato" - valores nos extremos da escala. Qualquer leitura fora dessas
// margens e tratada como toque em potencial (depois filtrada/validada).
#define TOUCH_RAW_MIN_VALID   50
#define TOUCH_RAW_MAX_VALID   4040

// Debounce entre toques aceitos (ms)
#define TOUCH_DEBOUNCE_MS 40

// Enderecos EEPROM para persistir a calibracao do touch
#define EEPROM_CAL_MAGIC_ADDR   0
#define EEPROM_CAL_DATA_ADDR    2
#define EEPROM_CAL_MAGIC_VALUE  0xC412

// ----------------------------------------------------------------------------
// Serial
// ----------------------------------------------------------------------------
#define SERIAL_BAUD 115200

// ----------------------------------------------------------------------------
// Paleta de cores (RGB565 - compativel com UTFT::setColor(word))
// ----------------------------------------------------------------------------
#define COLOR_BLACK    0x0000
#define COLOR_WHITE    0xFFFF
#define COLOR_RED      0xF800
#define COLOR_GREEN    0x07E0
#define COLOR_BLUE     0x001F
#define COLOR_YELLOW   0xFFE0
#define COLOR_CYAN     0x07FF
#define COLOR_MAGENTA  0xF81F
#define COLOR_GRAY     0x8410
#define COLOR_DARKGRAY 0x4208
#define COLOR_ORANGE   0xFDA0

// ----------------------------------------------------------------------------
// Estrutura de resultado de teste
// ----------------------------------------------------------------------------
enum TestResult { RESULT_PASS, RESULT_FAIL, RESULT_SKIPPED, RESULT_NOT_RUN };

struct TestRecord {
  const char *name;
  TestResult result;
  unsigned long durationMs;
};

#define TOTAL_TESTS 12  // 1..12 (sem contar "13 - completo" nem "0 - voltar")

#endif // CONFIG_H
