# Plano de Testes — TFT Hardware Tester (SSD1289 + Mega 2560)

Checklist de bancada para validar uma unidade de hardware (display + shield +
Mega). Marque cada linha como PASS / FAIL / N-A durante a execução real.

## Pré-requisitos

- [x] Arduino Mega 2560 reconhecido pelo PC (porta COM/tty visível).
- [x] Bibliotecas UTFT (e opcionalmente URTouch) instaladas manualmente — ver `README.md`.
- [x] `DefaultFonts.c` da UTFT corrigido com `const` (necessário em avr-gcc moderno).
- [x] Firmware compilado sem erros/avisos relevantes.
- [x] Firmware gravado com sucesso.
- [x] Serial Monitor aberto a 115200 baud.

## Sequência recomendada — resultado real de bancada (unidade do usuário)

| # | Teste | Critério de PASS | Resultado real |
|---|-------|-------------------|:---------:|
| 1 | Inicialização | Resolução relatada = `320x240`, controlador SSD1289 confirmado | ✅ PASS (331 ms) |
| 2 | Cores básicas | As 8 cores exibem corretamente, nome legível sobre cada uma | ✅ PASS (5878 ms) |
| 3 | Gradiente | Transições suaves R/G/B/RGB, sem faixas abruptas | ✅ PASS (7921 ms) |
| 4 | Pixels | Nenhum pixel apagado/preso visível na grade (verificação visual — UTFT não le GRAM) | ✅ PASS (412 ms) |
| 5 | Linhas | Todas as linhas retas, sem quebras; moldura fechada | ✅ PASS (244 ms) |
| 6 | Retângulos | Formas com proporções corretas, preenchimento uniforme | ✅ PASS (219 ms) |
| 7 | Círculos | Bordas suaves, preenchimento uniforme | ✅ PASS (282 ms) |
| 8 | Texto | Todos os tamanhos legíveis, sem corte nas bordas | ✅ PASS (463 ms) |
| 9 | Orientação | Portrait e Landscape corretos (só 2 orientações — limitação da UTFT, ver README) | ✅ PASS (3564 ms) |
| 10 | Padrão diagnóstico | Todos os elementos visíveis e no lugar esperado | ✅ PASS (285 ms) |
| 11 | Touch | Calibração + 4 cantos + centro dentro da tolerância; desenho livre acompanha o dedo | ✅ PASS (34190 ms, com interação do usuário) |
| 12 | Desempenho | Todos os tempos reportados > 0 e plausíveis | ✅ PASS (4989 ms) |
| 13 | Teste completo | Executa 1-12 sem travar; resumo bate com os resultados individuais | ✅ PASS (Total: 12 PASS / 0 FAIL / 0 SKIPPED) |

## Problemas encontrados e corrigidos durante esta bancada

1. **Biblioteca errada inicialmente (MCUFRIEND_kbv)**: tela sempre em
   branco, em 5 variantes de pinagem testadas. Causa: shield usa modo
   paralelo de 16 bits, não 8. Corrigido trocando para UTFT (ver
   `docs/hardware.md`).
2. **Touch sem resposta via biblioteca URTouch**: pino de IRQ (D2) não
   muda de estado ao tocar. Corrigido lendo o XPT2046 por SPI manual, sem
   depender de IRQ.
3. **Touch com eixos X/Y trocados**: toques caíam sistematicamente fora do
   alvo esperado após calibração "correta". Corrigido invertendo os
   comandos de canal (`0x90`/`0xD0`) na leitura bruta.

## Critérios gerais de reprovação (qualquer teste)

- Travamento do firmware (sem resposta a touch nem Serial por mais de ~30s
  fora das esperas de calibração de touch, que têm timeout de 20s por
  etapa).
- Reinício espontâneo do Arduino durante qualquer teste (sintoma de
  brownout — tente alimentação externa).

## Registro da unidade testada

- Data do teste: 2026-09-15
- Controlador: SSD1289 (confirmado — display acendeu corretamente com esta biblioteca/pinagem)
- Touch presente fisicamente (inspeção visual)? Sim
- Touch funcional (Teste 11 = PASS)? Sim (após correção de eixos e abandono do IRQ)
- Pinagem: RS=38, WR=39, CS=40, RST=41 (display); CLK=6, CS=5, DIN=4, DOUT=3 (touch, IRQ=2 não usado)
- Observações: nenhum defeito de hardware encontrado — todos os problemas
  iniciais eram de configuração de software (biblioteca/pinagem), não do
  display ou do shield em si.

Para registrar uma unidade diferente, copie a tabela acima e preencha com
seus próprios resultados.
