# Análise de Hardware

Este documento registra como o hardware foi identificado, incluindo o processo
real (pesquisa + testes de bancada + iteração), não apenas o resultado final.
A descoberta exigiu testar múltiplas hipóteses erradas no hardware real antes
de chegar à configuração correta — isso está documentado aqui de propósito,
porque as hipóteses descartadas (e o motivo de terem sido descartadas) são
úteis se você tiver uma unidade ligeiramente diferente.

## Resultado final (✅ CONFIRMADO EM BANCADA)

| Item | Valor |
|---|---|
| Shield | TFT LCD Mega Shield V2.2, fabricante **ElecFreaks** (texto "ElecFreaks.com 12/10/2012" serigrafado no verso) |
| Display | Painel com "LM32US01" serigrafado no vidro |
| Controlador | **SSD1289** |
| Resolução | 320×240 |
| Interface | Paralela de **16 bits verdadeiros** (não 8 bits) |
| Biblioteca correta | **UTFT** (Rinky-Dink Electronics / Henning Karlsen) |
| Touch | Resistivo, controlador **XPT2046**, lido por **SPI manual** (não pela biblioteca URTouch - ver seção 5) |

## 1. Por que a MCUFRIEND_kbv não funcionou

A primeira versão deste projeto usava a biblioteca MCUFRIEND_kbv (ver histórico
do repositório), que foi escolhida por ser a opção padrão da comunidade para
"shields Mega genéricos de controlador desconhecido" com barramento de 8 bits.
Essa era uma escolha tecnicamente razoável dado o que se sabia até então, mas
**testada no hardware real, nunca produziu imagem** — foram tentadas 5
variantes de pinagem de 8 bits (padrão, OpenSmart, `USE_MEGA_8BIT_SHIELD`,
`USE_MEGA_16BIT_SHIELD` da própria MCUFRIEND_kbv, e uma pinagem customizada
derivada de fóruns) e nenhuma acendeu a tela.

A causa raiz, descoberta só depois de obter o esquemático real do shield: **o
shield usa o display em modo paralelo de 16 bits genuíno**, com o barramento
de dados ocupando `PORTA` (D22-D29, byte alto) + `PORTC` (D30-D37, byte
baixo) do Mega2560 — pinos exclusivos do Mega, fora do header compatível com
Uno que a MCUFRIEND_kbv assume por padrão. Uma biblioteca de 8 bits
estruturalmente não consegue formar um comando válido de 16 bits para o
SSD1289 usando só a metade do barramento.

## 2. Como o controlador e a pinagem foram confirmados

Nesta ordem:

1. **Fórum Arduino** - post com o esquemático oficial em PDF do
   "TFT Mega Shield V2.2, new Version — Touch Screen · TFT LCD · SD Card"
   ([thread](https://forum.arduino.cc/t/arduino-tft-mega-shield-v2-2-schematic-as-pdf-file/1256159)),
   mostrando os buffers 74HC541 entre o conector do display e os pinos do
   Mega, e a pinagem RS=38/WR=39/CS=40/RESET=41.
2. **Exemplo de código real e público** no GitHub,
   [milstrike/TFT-LCD-Arduino](https://github.com/milstrike/TFT-LCD-Arduino),
   com um sketch funcional para este shield exato:
   ```cpp
   UTFT myGLCD(SSD1289, 38, 39, 40, 41);
   URTouch myTouch(6, 5, 4, 3, 2);
   ```
   Essa é a mesma pinagem de controle do esquemático — confirmação cruzada
   por duas fontes independentes.
3. **Teste real em bancada** (este projeto): a pinagem acima foi gravada no
   Arduino Mega físico do usuário e a tela **acendeu corretamente**,
   mostrando 4 retângulos coloridos (vermelho/verde/azul/amarelo) exatamente
   como esperado. Isso elevou a confirmação de "provável" para **confirmado**.

## 3. Touch: o IRQ não funciona, mas a leitura SPI funciona

O esquemático e o exemplo do GitHub indicam `URTouch(6, 5, 4, 3, 2)` -
CLK=6, CS=5, DIN=4, DOUT=3, IRQ=2. Testado em bancada:

- A biblioteca **URTouch**, que depende do pino de **IRQ (D2)** para saber
  quando há um toque (`dataAvailable()` lê esse pino), **nunca detectou
  nada** - o pino D2 não muda de estado ao tocar a tela, mesmo com
  `INPUT_PULLUP` e o chip de touch ativado manualmente via CS.
- Um teste manual de leitura SPI (bit-bang direto dos pinos CLK/CS/DIN/DOUT,
  ignorando IRQ) mostrou que os canais X/Y do XPT2046 **respondem
  perfeitamente** ao toque, com valores mudando de forma consistente.

**Conclusão prática**: os 4 pinos de dados do touch (CLK, CS, DIN, DOUT)
estão corretos; o IRQ não está, seja por não estar de fato conectado nesta
revisão do shield, seja por outro motivo não identificado. O firmware deste
projeto **não depende do IRQ** - ele faz *polling* contínuo via SPI manual e
filtra leituras inválidas pelos extremos de escala do ADC (0 ou ~4095 =
sem contato). Ver `firmware/tft_hardware_tester/test_touch.cpp`.

Outro detalhe encontrado em bancada: o **eixo X do touch corresponde ao eixo
vertical da tela** (e vice-versa) - ou seja, os canais X/Y do XPT2046 estão
trocados em relação à orientação landscape mostrada pelo controlador. Isso é
comum quando o filme resistivo é fisicamente portrait e só o conteúdo do
display é rotacionado para landscape. O firmware já compensa isso trocando
os comandos de canal na leitura (`0x90`/`0xD0` invertidos - ver comentário em
`readTouchRaw()`).

## 4. Por que a biblioteca não é a do Gerenciador de Bibliotecas

`UTFT` e `URTouch` (Henning Karlsen / Rinky-Dink Electronics) não estão
disponíveis no Gerenciador de Bibliotecas da Arduino IDE - a licença
(CC BY-NC-SA, não-comercial) impede a distribuição automática. É necessário
baixar manualmente. Ver `README.md` para o passo a passo. Este projeto usa
`URTouch` apenas como referência de pinagem (não para leitura de fato, pelo
motivo da seção 3) - por isso ela é opcional para compilar o firmware final
(só é citada aqui pela investigação), mas recomendamos instalá-la mesmo assim
caso quera comparar/depurar.

## 5. Limitação conhecida: apenas 2 orientações

A UTFT, para este controlador, expõe só `PORTRAIT` (0) e `LANDSCAPE` (1) via
`InitLCD()` - não há um modo "invertido" (180°) nativo na API pública. O
Teste 9 (Orientação) cobre as 2 orientações realmente disponíveis, e isso
está documentado no próprio código e no `README.md`.

## 6. Sobre o tamanho físico (2.4" vs 3.2")

O usuário descreveu o display como "2.4 polegadas". Múltiplas fontes sobre a
família "TFT_320QVT"/shields ElecFreaks V2.2 (incluindo o PDF de um
revendedor consultado durante o projeto) descrevem a unidade compatível como
**3.2 polegadas**. Isso não afeta o firmware (a resolução em pixels, 320×240,
é a mesma), mas vale confirmar o tamanho físico real se for relevante para
projetar um gabinete/case.

## 7. Fontes consultadas

- Fórum Arduino: [Arduino TFT Mega Shield V2.2 (schematic as PDF)](https://forum.arduino.cc/t/arduino-tft-mega-shield-v2-2-schematic-as-pdf-file/1256159) — esquemático oficial
- GitHub: [milstrike/TFT-LCD-Arduino](https://github.com/milstrike/TFT-LCD-Arduino) — exemplos reais e funcionais para este shield
- GitHub: [ivanseidel/UTFT](https://github.com/ivanseidel/UTFT) — mirror da biblioteca UTFT (código-fonte usado para confirmar o mapeamento PORTA/PORTC)
- GitHub: [f1rmb/URTouch](https://github.com/f1rmb/URTouch) — mirror da biblioteca URTouch
- Testes de bancada realizados diretamente no Arduino Mega 2560 físico deste projeto (compilação e gravação via `arduino-cli`, leitura de Serial via script)
