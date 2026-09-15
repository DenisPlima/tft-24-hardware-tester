# Troubleshooting

## Índice rápido

1. [Tela não liga / fica branca](#1-tela-não-liga--fica-branca)
2. [Cores erradas / trocadas](#2-cores-erradas--trocadas)
3. [Imagem cortada, deslocada ou espelhada](#3-imagem-cortada-deslocada-ou-espelhada)
4. [Touch não responde a nada](#4-touch-não-responde-a-nada)
5. [Touch responde só em parte da tela / eixos trocados](#5-touch-responde-só-em-parte-da-tela--eixos-trocados)
6. [Erros de compilação comuns](#6-erros-de-compilação-comuns)
7. [Se você tem um shield/display diferente deste](#7-se-você-tem-um-shielddisplay-diferente-deste)

---

## 1. Tela não liga / fica branca

Este projeto já passou por esse problema exato durante o desenvolvimento (ver
[`hardware.md`](hardware.md) para a história completa) - a causa raiz foi
usar uma biblioteca de 8 bits (MCUFRIEND_kbv) num shield que precisa de 16
bits de verdade. Se você está vendo tela branca com **este firmware** (que já
usa a biblioteca certa, UTFT), verifique nesta ordem:

1. **Placa/processador errados na IDE.** Confirme *Ferramentas → Placa →
   Arduino Mega or Mega 2560* e *Ferramentas → Processador → ATmega2560*.
2. **Biblioteca UTFT não instalada corretamente**, ou instalada a versão
   errada (existe uma UTFT para PIC32/outras arquiteturas - confirme que
   pegou a que suporta AVR/Mega). Veja `README.md` para o link exato usado
   neste projeto.
3. **Shield mal encaixado.** Reencaixe pressionando os 4 cantos igualmente.
4. **Alimentação insuficiente pela USB.** Tente alimentar o Mega por fonte
   externa 7-12V enquanto testa.
5. Rode o **Teste 1 (Inicialização)** e confira a resolução relatada no
   Serial Monitor (115200 baud) - se vier `320x240`, a comunicação básica
   está OK e o problema é outro (item 2 ou 3 abaixo).

## 2. Cores erradas / trocadas

Se as cores primárias aparecerem sistematicamente trocadas (vermelho vira
azul, etc.) em todos os testes, isso é uma característica do painel
específico (ordem de bits RGB/BGR), não um bug do firmware - aceitável para
fins de diagnóstico, já que o objetivo é confirmar que os pixels respondem.

## 3. Imagem cortada, deslocada ou espelhada

Rode o **Teste 9 (Orientação)**. Note que esta biblioteca só suporta 2
orientações (Portrait e Landscape) para este controlador - não existe um
modo "invertido" nativo na UTFT para o SSD1289. Se a imagem parecer
espelhada horizontalmente dentro de uma mesma orientação, isso indicaria um
problema de fiação na *ordem* dos bits do barramento de 16 bits - pouco
provável já que a pinagem é fixa e gerenciada pela própria biblioteca, mas,
se acontecer, verifique se não há dano físico no header do Mega (pino
dobrado/faltando entre D22-D37).

## 4. Touch não responde a nada

1. Confirme fisicamente que existe o filme resistivo sobre o vidro (ver
   `hardware.md` seção 1).
2. No **Teste 11**, observe a "leitura crua sem toque esperado" impressa no
   Serial antes da calibração começar. Se os valores de `x`/`y` forem
   sempre exatamente `0` ou sempre `4095` (extremos da escala do ADC) mesmo
   tocando forte no vidro, os pinos do touch (`TOUCH_CLK/CS/DIN/DOUT` em
   `config.h`) provavelmente não são os do seu shield.
3. Se você tiver uma revisão diferente do shield, o `T_IRQ` pode funcionar
   nela (não funcionou na unidade usada para desenvolver este projeto) - mas
   como o firmware não depende dele, isso não deveria importar.
4. Se nada disso resolver, use um multímetro em modo de continuidade entre
   os 4 fios do conector de touch (geralmente uma fita FFC saindo do vidro)
   e os pinos do Mega, para confirmar fisicamente a pinagem.

## 5. Touch responde só em parte da tela / eixos trocados

O Teste 11 sempre recalibra do zero a cada execução (2 toques: canto
superior-esquerdo e inferior-direito). Se mesmo assim os toques caírem
sistematicamente fora do esperado (não apenas um pouco impreciso, mas
claramente na posição errada, como se X e Y estivessem invertidos), o
firmware já tenta compensar a troca de eixos conhecida deste hardware (ver
`hardware.md` seção 3) - se sua unidade for diferente e ainda assim os
eixos parecerem trocados, edite `readTouchRaw()` em `test_touch.cpp` e
inverta de volta `0x90`/`0xD0`.

## 6. Erros de compilação comuns

- **`UTFT.h: No such file or directory`** — biblioteca UTFT não instalada.
  Veja `README.md`, ela não está no Gerenciador de Bibliotecas.
- **`variable 'SmallFont' must be const in order to be put into read-only
  section`** — versões recentes do avr-gcc exigem `const` em arrays
  `PROGMEM`. A UTFT original (2010s) não tem isso. Edite
  `<pasta_da_biblioteca>/UTFT/DefaultFonts.c` e adicione `const` antes de
  `fontdatatype SmallFont[...]`, `BigFont[...]` e `SevenSegNumFont[...]`.
  Isso foi necessário e testado durante o desenvolvimento deste projeto.
- **Erros de "multiple definition"** — confirme que todos os arquivos do
  firmware estão soltos dentro de `firmware/tft_hardware_tester/`, no mesmo
  nível do `.ino` principal (sem subpastas).

## 7. Se você tem um shield/display diferente deste

Este firmware foi validado especificamente contra: shield ElecFreaks "TFT
LCD Mega Shield V2.2" + display "LM32US01" (controlador SSD1289). Se o seu
hardware for parecido mas não idêntico (outra revisão, outro fabricante
vendendo uma placa semelhante), a pinagem de controle (RS/WR/CS/RST) pode
ser diferente. Consulte `hardware.md` para o processo usado para descobrir a
pinagem correta (esquemático → exemplo de código real → teste em bancada) -
o mesmo processo se aplica a qualquer shield Mega de controlador
desconhecido.
