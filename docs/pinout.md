# Pinout — TFT LCD Mega Shield V2.2 (ElecFreaks) + Arduino Mega 2560

> Ver [`hardware.md`](hardware.md) para como cada valor abaixo foi
> confirmado (esquemático oficial + exemplo real da comunidade + teste em
> bancada). Este documento é a referência rápida de pinos.

## Display (controlador SSD1289) — ✅ confirmado em bancada

O barramento de dados é gerenciado internamente pela biblioteca **UTFT** -
não é definido no sketch. Só os 4 pinos de controle são passados no
construtor:

```cpp
UTFT tft(SSD1289, 38, 39, 40, 41);
//         RS ^   WR ^  CS ^  RST ^
```

| Sinal | Pino Mega |
|-------|:---------:|
| RS (Register Select) | D38 |
| WR (Write)            | D39 |
| CS (Chip Select)      | D40 |
| RESET                 | D41 |
| RD (Read)              | não utilizado (shield somente-escrita) |

### Barramento de dados (16 bits) — interno à biblioteca UTFT

| Byte | Porta AVR | Pinos Mega |
|------|:---------:|:----------:|
| Alto (bits 8-15) | PORTA | D22, D23, D24, D25, D26, D27, D28, D29 (bit0→D22 ... bit7→D29) |
| Baixo (bits 0-7)  | PORTC | D30, D31, D32, D33, D34, D35, D36, D37 (bit0→D37 ... bit7→D30 - ordem fisicamente invertida no Mega, tratada automaticamente pela UTFT) |

Estes são pinos **exclusivos do Mega** (fora do header compatível com Uno) -
por isso uma biblioteca pensada só para Uno/8-bit nunca funcionaria aqui.

## Touch (controlador XPT2046) — ✅ confirmado em bancada

```cpp
// Pinos de referencia (mesmos do esquematico e do exemplo da comunidade):
// CLK=6  CS=5  DIN=4  DOUT=3  IRQ=2
```

| Sinal | Pino Mega | Observação |
|-------|:---------:|------------|
| T_CLK  | D6 | clock SPI |
| T_CS   | D5 | chip select |
| T_DIN  | D4 | MOSI (Arduino → touch) |
| T_DOUT | D3 | MISO (touch → Arduino) |
| T_IRQ  | D2 | **presente no esquemático, mas não respondeu em bancada neste shield** - o firmware não depende dele (ver abaixo) |

**Importante**: o firmware **não usa a biblioteca URTouch** para ler o touch
(ela depende do pino IRQ, que não funciona nesta unidade). Em vez disso, ele
faz leitura SPI manual (bit-bang) diretamente em `test_touch.cpp`, com
*polling* contínuo e filtro por faixa válida do ADC. Além disso, os canais
X/Y do XPT2046 estão **trocados** em relação ao padrão "de livro" (o filme
resistivo é fisicamente portrait, o conteúdo do display é landscape) - isso
já está compensado no código (`readTouchRaw()`).

## SD Card

O shield também expõe um slot de SD card (não utilizado por este firmware).
Segue o barramento SPI padrão do Mega (pinos 50-53: MISO/MOSI/SCK/SS) mais um
pino de CS dedicado ao cartão, roteado através de um dos buffers 74HC541 do
esquemático. Fora do escopo deste projeto.

## Alimentação

O shield toma alimentação diretamente dos pinos 5V/3.3V/GND do header do
Mega. Há um regulador `AMS1117-3.3V` na própria placa do shield, então não é
preciso fornecer 3.3V externamente.
