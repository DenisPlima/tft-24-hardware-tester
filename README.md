# TFT 2.4"/3.2" Hardware Tester

Programa de diagnóstico e teste de hardware para display TFT 320×240
(controlador **SSD1289**) sobre o shield **TFT LCD Mega Shield V2.2
(ElecFreaks)** + **Arduino Mega 2560**.

Testa inicialização, cores, gradientes, pixels, linhas, retângulos,
círculos, texto, orientação, um padrão de diagnóstico completo, touch
resistivo (controlador XPT2046) e desempenho — via menu gráfico na própria
tela (com botões grandes para touch) **e** via menu no Serial Monitor.

**Todos os 12 testes passaram em bancada, no hardware real do usuário, com o
firmware deste repositório.** Ver a seção [Como este projeto chegou até
aqui](#como-este-projeto-chegou-até-aqui) para a história completa — vale a
leitura se você tem um shield parecido mas não idêntico.

## Sumário

- [Hardware](#hardware)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Bibliotecas necessárias](#bibliotecas-necessárias-instalação-manual)
- [Como compilar e gravar](#como-compilar-e-gravar)
- [Como usar](#como-usar)
- [Como testar cada recurso](#como-testar-cada-recurso)
- [Calibração do touch](#calibração-do-touch)
- [Resultados da bancada de testes](#resultados-da-bancada-de-testes)
- [Como este projeto chegou até aqui](#como-este-projeto-chegou-até-aqui)
- [Limitações conhecidas](#limitações-conhecidas--decisões-de-projeto)

## Hardware

| Item | Valor |
|---|---|
| Placa | Arduino Mega 2560 |
| Shield | TFT LCD Mega Shield V2.2 — **ElecFreaks** |
| Display | 320×240, serigrafado "LM32US01" |
| Controlador do display | **SSD1289**, interface paralela de **16 bits** |
| Controlador de touch | **XPT2046** (resistivo), lido por SPI manual |

Ver [`docs/hardware.md`](docs/hardware.md) para a análise completa (o que é
confirmado, como foi confirmado, e as fontes) e
[`docs/pinout.md`](docs/pinout.md) para a tabela de pinos.

## Estrutura do projeto

```
tft-24-hardware-tester/
├── README.md
├── docs/
│   ├── hardware.md            (análise do hardware + historico da descoberta)
│   ├── pinout.md               (tabela de pinos confirmada)
│   └── troubleshooting.md      (solução de problemas)
├── firmware/
│   └── tft_hardware_tester/
│       ├── tft_hardware_tester.ino   (setup/loop, menus, despacho de testes)
│       ├── config.h                  (pinos, cores, flags de recursos)
│       ├── state.h / state.cpp       (estado global, contador de testes)
│       ├── ui.h / ui.cpp             (helpers de desenho compartilhados)
│       ├── test_graphics.h/.cpp      (Testes 1-10)
│       ├── test_touch.h/.cpp         (Teste 11)
│       └── test_performance.h/.cpp   (Teste 12)
└── tests/
    └── test_plan.md               (plano de testes / checklist de bancada)
```

## Bibliotecas necessárias (instalação manual)

`UTFT` e `URTouch` (Henning Karlsen / Rinky-Dink Electronics) **não estão no
Gerenciador de Bibliotecas** da Arduino IDE — a licença (CC BY-NC-SA,
uso não-comercial) impede distribuição automática. Instale manualmente:

1. Baixe o ZIP da **UTFT**: [github.com/ivanseidel/UTFT](https://github.com/ivanseidel/UTFT)
   (`Code → Download ZIP`).
2. Extraia para `Documentos/Arduino/libraries/UTFT` (a pasta deve conter
   `UTFT.h` diretamente, sem subpasta extra).
3. **Correção necessária** (avr-gcc moderno exige `const` em arrays
   `PROGMEM`, a biblioteca original não tem): abra
   `Documentos/Arduino/libraries/UTFT/DefaultFonts.c` e adicione `const`
   antes de `fontdatatype SmallFont[...]`, `fontdatatype BigFont[...]` e
   `fontdatatype SevenSegNumFont[...]` (3 linhas). Sem isso a compilação
   falha com `variable must be const in order to be put into read-only
   section`.
4. (Opcional, só para referência/depuração) Baixe a **URTouch**:
   [github.com/f1rmb/URTouch](https://github.com/f1rmb/URTouch), mesmo
   processo, pasta `Documentos/Arduino/libraries/URTouch`. Este firmware
   **não usa** a URTouch para ler o touch de fato (ver
   `docs/hardware.md` seção 3) — ela não é uma dependência obrigatória
   para compilar.

Nenhuma outra biblioteca é necessária (o firmware não usa Adafruit_GFX,
MCUFRIEND_kbv, TouchScreen ou EEPROM externas — só a EEPROM interna do Mega,
via `<EEPROM.h>`, que já vem com a IDE).

## Como compilar e gravar

### Pela Arduino IDE

1. Abra `firmware/tft_hardware_tester/tft_hardware_tester.ino`.
2. **Ferramentas → Placa → Arduino AVR Boards → Arduino Mega or Mega 2560**.
3. **Ferramentas → Processador → ATmega2560 (Mega 2560)**.
4. **Ferramentas → Porta** → selecione a porta serial do seu Mega.
5. **Verificar** (✓) e depois **Carregar** (→).

### Por linha de comando (arduino-cli), opcional

```bash
arduino-cli core install arduino:avr
# UTFT precisa ser instalada manualmente (ver secao acima) em
# Documentos/Arduino/libraries/UTFT antes deste passo
arduino-cli compile --fqbn arduino:avr:mega:cpu=atmega2560 firmware/tft_hardware_tester
arduino-cli upload -p COM_PORTA --fqbn arduino:avr:mega:cpu=atmega2560 firmware/tft_hardware_tester
```

## Como usar

Ao ligar, o display mostra o menu principal (grade de botões grandes,
tocáveis) e o Serial Monitor (115200 baud) mostra o mesmo menu em texto:

```
================================
TFT HARDWARE TESTER
1 - Inicializacao
2 - Cores
3 - Gradiente
4 - Pixels
5 - Linhas
6 - Retangulos
7 - Circulos
8 - Texto
9 - Orientacao
10 - Padrao diagnostico
11 - Touch
12 - Desempenho
13 - Teste completo
0 - Voltar
================================
Escolha uma opcao:
```

Digite o número da opção + Enter no Serial, **ou** toque no botão
correspondente na tela. Os dois funcionam ao mesmo tempo, sempre.

**Dica**: rode o Teste 11 (Touch) uma vez logo no início — ele calibra e
salva na EEPROM, o que deixa os toques no menu principal muito mais
precisos daí em diante (antes da primeira calibração, o menu usa valores
de fábrica que podem não bater exatamente com o seu painel).

## Como testar cada recurso

| Nº | Teste | O que observar |
|---|---|---|
| 1 | Inicialização | Tela limpa, resolução `320x240`, controlador `SSD1289`. |
| 2 | Cores | 8 telas cheias em sequência, cada uma com o nome da cor escrito. |
| 3 | Gradiente | Faixas contínuas R/G/B/RGB, horizontais e verticais. |
| 4 | Pixels | Pontos nos cantos/centro/bordas + grade verde esparsa — procure pixels apagados/presos. |
| 5 | Linhas | Horizontais, verticais, diagonais cruzadas e moldura. |
| 6 | Retângulos | Vazio, preenchido, quadrado com molduras concêntricas. |
| 7 | Círculos | Preenchido, vazio, concêntricos. |
| 8 | Texto | Tamanhos pequeno/médio/grande, números, símbolos, cores, fundo colorido. |
| 9 | Orientação | Portrait e Landscape (só 2 — ver [Limitações](#limitações-conhecidas--decisões-de-projeto)) com grade + eixos rotulados. |
| 10 | Padrão diagnóstico | Tela de referência única (grade, molduras, círculos nos cantos, cruz central, texto nos 4 cantos, barra de cores). |
| 11 | Touch | Calibração (2 toques) → 4 cantos → centro → desenho livre com botão LIMPAR. Ver seção abaixo. |
| 12 | Desempenho | Tempos de fill/linhas/círculos/texto no Serial e na tela — sem PASS/FAIL absoluto, é referência comparativa. |
| 13 | Teste completo | Roda 1-12 em sequência, com resumo final no Serial. |

## Calibração do touch

1. Escolha **11 - Touch**.
2. Toque no alvo (mira amarela) perto do canto **superior-esquerdo**, depois
   no canto **inferior-direito**. Isso salva a calibração na EEPROM
   (persiste entre religadas).
3. Toque nos **4 cantos** e no **centro** conforme pedido — cada um fica
   verde se detectado dentro da tolerância, vermelho se fora.
4. Modo de **desenho livre**: toque para desenhar, toque em **LIMPAR** para
   apagar, toque em **VOLTAR** (ou `q` no Serial) para sair.
5. Se alguma etapa não responder em ~20s, ela segue em frente sozinha (ou
   envie `s` no Serial para pular manualmente).

## Resultados da bancada de testes

Execução real do **Teste 13 (completo)** no hardware do usuário:

```
1  - Inicializacao:      PASS (331 ms)
2  - Cores basicas:      PASS (5878 ms)
3  - Gradiente de cores: PASS (7921 ms)
4  - Pixels:             PASS (412 ms)
5  - Linhas:              PASS (244 ms)
6  - Retangulos:          PASS (219 ms)
7  - Circulos:            PASS (282 ms)
8  - Texto:               PASS (463 ms)
9  - Orientacao:          PASS (3564 ms)
10 - Padrao diagnostico: PASS (285 ms)
11 - Touch:               PASS (34190 ms — inclui interacao do usuario)
12 - Desempenho:          PASS (4989 ms)

Total PASS: 12 | FAIL: 0 | SKIPPED: 0
```

Números de desempenho do Teste 12 (referência, não é "rápido" — biblioteca
UTFT sem aceleração de hardware, em AVR 16MHz):

| Métrica | Valor |
|---|---|
| Fill de tela inteira | ~115 ms (≈8.7 "FPS" de fill completo) |
| 100 linhas | ~836 ms |
| 100 círculos | ~357 ms |
| 50× atualização de texto | ~2295 ms |

## Como este projeto chegou até aqui

Resumo honesto do processo, porque as etapas descartadas têm valor caso seu
hardware seja parecido mas não idêntico (história completa em
[`docs/hardware.md`](docs/hardware.md)):

1. A análise inicial de hardware (sem fotos, só pesquisa) apontou a
   biblioteca **MCUFRIEND_kbv** como escolha tecnicamente razoável para um
   "shield Mega genérico de controlador desconhecido, 8 bits". Essa
   suposição sobre a interface (8 bits) acabou errada para este shield
   específico.
2. Testado no Arduino Mega real do usuário, a MCUFRIEND_kbv **nunca**
   produziu imagem, mesmo testando 5 variantes de pinagem diferentes e 10
   IDs de controlador conhecidos.
3. Fotos do hardware revelaram o modelo exato: shield **ElecFreaks TFT LCD
   Mega Shield V2.2** + display **LM32US01**.
4. Uma busca pelo esquemático oficial (PDF) e por um exemplo de código real
   no GitHub confirmaram: o shield usa o display em **modo paralelo de 16
   bits genuíno** (não 8 bits), com a biblioteca **UTFT** e controlador
   **SSD1289** — pinos RS=38, WR=39, CS=40, RST=41.
5. Essa configuração foi gravada no hardware real e **funcionou de
   primeira** (tela mostrou as 4 cores esperadas do teste).
6. O touch (XPT2046) tinha pinagem também documentada
   (CLK=6, CS=5, DIN=4, DOUT=3, IRQ=2), mas o **IRQ não respondeu** em
   bancada. Um teste de leitura SPI manual confirmou que os 4 outros pinos
   funcionam — o firmware foi então escrito para ler o touch **sem depender
   do IRQ**, por *polling* contínuo.
7. Um problema de eixos X/Y trocados no touch (comum quando o filme
   resistivo é portrait e o display é rotacionado para landscape) foi
   identificado e corrigido também em bancada.
8. Todo o firmware foi reescrito da MCUFRIEND_kbv para a UTFT, e o
   **Teste 13 completo passou 12/12** no hardware real.

## Limitações conhecidas / decisões de projeto

- **Só 2 orientações no Teste 9** (Portrait/Landscape), não as 4 pedidas
  originalmente — a UTFT não expõe um modo invertido (180°) para este
  controlador via API pública.
- **Testes gráficos (Pixels, Linhas, Retângulos, Círculos) são puramente
  visuais** — a UTFT não oferece leitura de volta do GRAM (`readPixel`),
  diferente do que a MCUFRIEND_kbv teria oferecido caso funcionasse neste
  hardware. Não há verificação automática de pixel a pixel; o Serial deixa
  isso explícito em cada teste afetado.
- **O "Teste de comunicação"** da especificação original foi incorporado ao
  **Teste 1 (Inicialização)** — o menu de 13 opções não reserva um número
  separado para ele.
- **Desempenho não tem limiar de PASS/FAIL fixo** — reporta os números para
  comparação, já que não existe um "mínimo aceitável" universal para este
  hardware específico.
- **O ID do controlador não é lido em tempo real** — diferente da
  MCUFRIEND_kbv (que teria uma função `readID()` caso funcionasse aqui), a
  UTFT não oferece essa leitura. O nome do controlador (`SSD1289`) é fixo em
  `config.h`, porque foi **confirmado em bancada**, não porque foi assumido
  às cegas.

## Licença

O código deste repositório (firmware e documentação) está sob licença
[MIT](LICENSE). As bibliotecas de terceiros necessárias para compilar
(`UTFT`, `URTouch`) **não** são MIT — são CC BY-NC-SA (uso não-comercial),
mantidas por Henning Karlsen / Rinky-Dink Electronics, e precisam ser
instaladas separadamente (ver [Bibliotecas necessárias](#bibliotecas-necessárias-instalação-manual)).
Respeite a licença delas se for redistribuir ou usar comercialmente.
