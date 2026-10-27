# Mini-Satélite Educacional — Projeto Final ENG4033 (Turma B, 2026.2)

Projeto do grupo para o Projeto Final de Microcontroladores (Prof. Jan K. S.).
Tema: **mini-satélite espacial educacional** — um satélite que gira, orienta seus painéis solares para a luz, se comunica por código Morse óptico e é controlado por joystick com uma interface gráfica que mostra seu estado.

| Parte | Responsável | Resumo |
|---|---|---|
| 1 | Lucas | Motor que gira o satélite e controle/leitura do ângulo |
| 2 | Gabriel | Comunicação Morse óptica (transmissor e receptor de luz) |
| 3 | Luca | Painéis solares laterais com inclinação por servo (rastreador solar) |
| 4 | Rafael | Joystick de controle + interface UX/UI com os dados do satélite |

---

## Requisitos do professor (valem para o grupo todo)

- Microcontrolador + componentes/sensores + programação.
- Interface gráfica para o usuário (web, LCD, TFT, Tkinter etc.).
- Montagem apresentável do produto (madeira/MDF, impressão 3D etc.).
- Código no **GitHub** com documentação e **diagrama do circuito no README**.
- Uso responsável de IA: cada um precisa entender e explicar o próprio código (há **avaliação oral semanal**).
- **Vídeo de até 5 min**: ideia → desenvolvimento → funcionamento.
- Metas semanais individuais no **Trello** (instalar no celular e ativar notificações; não mandar e-mail).

### Cronograma geral

| Data | Meta |
|---|---|
| 27/10 | Especificação do projeto |
| 03/11 | 1. Protótipo funcional |
| 10/11 | 2. Evolução do protótipo |
| 17/11 | 3. Integração inicial |
| 24/11 | 4. Refino |
| 01/12 | 5. Integração final + entrega (código, README, vídeo) |

### Pontuação

| Atividade | Pontos |
|---|---|
| Pontualidade | 40 XP (×5) |
| Acompanhamento das metas | 160 XP (×5) |
| Entrega do projeto + vídeo | 3000 XP |

---

## Arquitetura geral (decidir na primeira semana)

**Proposta:** um único Arduino no satélite, ligado por USB/Serial ao computador que roda a interface do Rafael.

```
 [Joystick] ──┐
              ├── Arduino ── Serial (USB) ── Interface no PC (Python)
 [Motor] ─────┤
 [Servo + LDRs + painel] ──┤
 [LED Morse TX + LDR Morse RX] ──┘
```

### Regras para o código juntar sem dor

1. **Proibido `delay()` no código final.** Tudo com `millis()` (máquinas de estado). Motor, servo e Morse dependem de tempo e travariam um ao outro.
2. Cada parte em seu próprio módulo (`motor.h/.cpp`, `morse.h/.cpp`, `solar.h/.cpp`, `joystick.h/.cpp`) com duas funções: `xxxSetup()` e `xxxLoop()`. O `loop()` principal só chama os `xxxLoop()`.
3. Uma única rotina lê a Serial e repassa cada comando para o módulo certo (pelo prefixo da mensagem).
4. **GND comum** entre Arduino, fonte externa, motor e servo.

### Mapa de pinos sugerido (Arduino Uno)

O Uno tem só 6 entradas analógicas e o projeto usa exatamente 6. Se faltar pino, trocar para Arduino Mega.

| Pino | Uso | Parte |
|---|---|---|
| A0 | LDR esquerdo do painel | 3 |
| A1 | LDR direito do painel | 3 |
| A2 | Tensão do painel solar (via divisor 2:1) | 3 |
| A3 | LDR receptor Morse | 2 |
| A4 | Joystick eixo X | 4 |
| A5 | Joystick eixo Y | 4 |
| D2 | Botão do joystick | 4 |
| D3 | LED/laser transmissor Morse | 2 |
| D4–D7 | Motor de passo (IN1–IN4 do ULN2003) | 1 |
| D8 | Buzzer (eco sonoro do Morse, opcional) | 2 |
| D9 | Servo dos painéis | 3 |
| D11 | Sensor de posição zero do motor (fim de curso/hall, opcional) | 1 |

Obs.: a biblioteca `Servo` desativa o PWM dos pinos 9 e 10 no Uno.

### Protocolo Serial (115200 baud, uma mensagem por linha, terminada em `\n`)

Arduino → PC (status a cada ~200 ms, e mensagens Morse quando chegarem):

| Mensagem | Exemplo | Parte |
|---|---|---|
| `MOT;ang=<graus>` | `MOT;ang=135` | 1 |
| `SOL;ang=<graus>;v=<volts>;modo=<A/M/R>` | `SOL;ang=72;v=4.31;modo=A` | 3 |
| `MRS;rx=<texto>` | `MRS;rx=SOS` | 2 |
| `JOY;x=<0-1023>;y=<0-1023>;bt=<0/1>` | `JOY;x=512;y=830;bt=0` | 4 |

PC → Arduino (comandos da interface):

| Mensagem | Exemplo | Parte |
|---|---|---|
| `MOT;ang=<graus>` | `MOT;ang=90` | 1 |
| `SOL;modo=<A/M/R>;ang=<graus>` | `SOL;modo=M;ang=90` | 3 |
| `MRS;tx=<texto>` | `MRS;tx=OLA TERRA` | 2 |

O formato fica fechado na semana 1. Qualquer mudança depois precisa ser avisada ao grupo, principalmente ao Rafael.

---

## Parte 1 — Motor que gira + ângulo (Lucas)

**Função:** girar o corpo do satélite e saber sempre em que ângulo ele está. Recebe o ângulo-alvo do joystick/interface e informa o ângulo atual.

**Hardware**
- Motor de passo **28BYJ-48 + driver ULN2003** (recomendado: o ângulo é conhecido pela contagem de passos, sem encoder).
  - Alternativa: motor DC + ponte H (L298N/L9110) + encoder ou potenciômetro no eixo para medir o ângulo.
- Sensor de posição zero (fim de curso, sensor hall + ímã ou sensor óptico) para "zerar" o ângulo ao ligar.
- Fonte 5 V externa para o motor.
- Base giratória: rolamento/disco giratório (lazy susan) e acoplamento do eixo.

**Software**
- Conversão graus ↔ passos (28BYJ-48 ≈ 2048 passos por volta no modo full-step).
- Movimento **não bloqueante** (biblioteca `AccelStepper` ou máquina de estados com `millis()`).
- Rotina de *homing* (procurar o zero ao ligar).
- Limitar a rotação (ex.: −180° a +180°) para **não enrolar os fios** do servo, LDRs e LED que estão no satélite.
- Comandos: ir para um ângulo, ou girar enquanto o joystick estiver inclinado.
- Enviar `MOT;ang=...` periodicamente.

**Metas**
| Semana | Meta |
|---|---|
| 03/11 | Motor girando para um ângulo pedido pela Serial |
| 10/11 | Movimento não bloqueante, homing e limites de rotação |
| 17/11 | Integração com o joystick/interface (Rafael) |
| 24/11 | Base giratória montada, ajuste de velocidade/aceleração |
| 01/12 | Integração final + documentação |

**Depende de:** Rafael (comandos do joystick). **Afeta:** Luca (a rotação muda a direção da luz nos painéis, e os fios precisam aguentar o giro).

---

## Parte 2 — Morse + sensores, envia e recebe (Gabriel)

**Função:** comunicação óptica em código Morse. Um emissor de luz transmite mensagens e um sensor de luz recebe e decodifica mensagens.

**Hardware**
- Transmissor: LED de alto brilho ou módulo laser (KY-008) + resistor.
- Receptor: LDR + resistor 10 kΩ (divisor) ou módulo sensor de luz com saída digital.
- Buzzer para "ouvir" o Morse (opcional, ajuda muito no vídeo).
- Botão para digitar Morse à mão (opcional).
- **Tubo/canudo preto em volta do LDR receptor** para bloquear luz ambiente e a lanterna da Parte 3.

**Software**
- Tabela Morse (A–Z, 0–9) e conversão texto → pontos/traços e pontos/traços → texto.
- Temporização padrão com unidade `u` (ex.: 150 ms): ponto = 1u, traço = 3u, espaço entre símbolos = 1u, entre letras = 3u, entre palavras = 7u.
- Transmissão **não bloqueante** (fila de símbolos + `millis()`).
- Recepção: medir duração de luz acesa/apagada com `millis()`, com limiar calibrado (ou adaptativo) para separar aceso de apagado.
- Receber texto da interface (`MRS;tx=...`) e enviar mensagem decodificada (`MRS;rx=...`).

**Metas**
| Semana | Meta |
|---|---|
| 03/11 | Transmitir uma mensagem fixa em Morse pelo LED |
| 10/11 | Receber e decodificar Morse pelo LDR (calibração do limiar) |
| 17/11 | Texto vindo da interface e mensagem decodificada indo para a interface |
| 24/11 | Robustez a luz ambiente (blindagem, limiar adaptativo), buzzer |
| 01/12 | Integração final + documentação |

**Depende de:** Rafael (envio/exibição das mensagens). **Atenção com:** Luca (a lanterna do rastreador solar pode interferir no LDR do Morse).

---

## Parte 3 — Painel solar com inclinação do servo (Luca)

**Função:** as laterais do satélite têm mini painéis solares presos a um servo. Dois LDRs comparam de onde vem mais luz e o servo inclina os painéis para ficarem de frente para a fonte de luz (o "sol", uma lanterna). A tensão gerada pelo painel é medida e mostrada na interface.

**Modos**
- **A — Automático:** segue a luz sozinho.
- **M — Manual:** ângulo definido pela interface/joystick.
- **R — Recolhido:** painéis fechados numa posição fixa.

**Hardware**
- 1 ou 2 servos SG90/MG90S (1 basta se as duas laterais se moverem juntas por um eixo).
- 2 LDRs + 2 resistores 10 kΩ (divisores em A0/A1), montados um de cada lado da placa, com uma pequena aba separadora entre eles.
- Mini painel solar 5–6 V.
- Divisor 2:1 (10 kΩ + 10 kΩ) para ler a tensão do painel em A2 sem passar de 5 V.
- Capacitor ~470 µF na alimentação do servo. Fonte 5 V externa se houver mais de um servo ou se a fonte for compartilhada com o motor.
- Opcional: sensor INA219 para medir corrente e potência.
- Lanterna ou luminária para os testes (a luz do teto é fraca demais).

**Software**
- Leitura dos LDRs com média móvel.
- Controle: se `|LDR_esq − LDR_dir| > zona_morta`, move o servo 1° na direção da luz (evita tremedeira).
- Movimento suave e não bloqueante do servo com `millis()`.
- Limites mecânicos do ângulo (ex.: 20°–160°).
- Leitura da tensão do painel: `V = leitura × 5.0 / 1023 × 2`.
- Calibração dos LDRs (diferença entre eles) salva na **EEPROM**.
- Comandos `SOL;modo=...;ang=...` e envio de `SOL;ang=...;v=...;modo=...`.

**Metas**
| Semana | Meta |
|---|---|
| 03/11 | Servo + 2 LDRs na protoboard: painel seguindo a luz com zona morta |
| 10/11 | Leitura da tensão do painel, movimento suave, modos A/M/R, calibração na EEPROM |
| 17/11 | Protocolo Serial com a interface (Rafael) e teste junto com o motor (Lucas) |
| 24/11 | Montagem mecânica das laterais (suporte, eixo, articulação), ajuste de limites |
| 01/12 | Integração final + documentação |

**Depende de:** Rafael (modo e ângulo manual). **Integração com:** Lucas (quando o satélite gira, o rastreador corrige sozinho; ótimo para o vídeo) e Gabriel (cuidado com a lanterna atrapalhando o Morse).

---

## Parte 4 — Joystick e controle com interface (Rafael)

**Função:** controlar o satélite com joystick e mostrar ao usuário tudo o que está acontecendo: ângulo do satélite, mensagens Morse, estado dos painéis. A interface também envia comandos.

**Hardware**
- Módulo joystick analógico (KY-023) ou joystick shield.
- (Opcional) LCD 16×2 I2C para mostrar o estado no próprio satélite/base.

**Software (Arduino)**
- Leitura de X, Y e botão, com zona morta no centro.
- Mapeamento: X → girar o satélite (Parte 1); Y → inclinar os painéis no modo manual (Parte 3); botão → alternar o modo dos painéis ou enviar uma mensagem Morse.
- Envio de `JOY;x=...;y=...;bt=...`.

**Software (PC — interface)**
- Python com **Tkinter** (ou Flask para página web) + `pyserial`.
- Thread/loop que lê a Serial e separa as mensagens pelo prefixo (`MOT`, `SOL`, `MRS`, `JOY`).
- Tela com:
  - indicador visual do ângulo do satélite (bússola/mostrador);
  - ângulo e tensão dos painéis + modo atual + gráfico da tensão ao longo do tempo;
  - histórico de mensagens Morse recebidas e campo para digitar e enviar mensagens;
  - botões de modo dos painéis (A/M/R) e campo de ângulo manual;
  - status da conexão Serial.
- (Opcional) log em arquivo/CSV das mensagens e da tensão gerada.

**Metas**
| Semana | Meta |
|---|---|
| 03/11 | Protótipo da interface (layout) + leitura do joystick no Arduino |
| 10/11 | Comunicação Serial PC ↔ Arduino funcionando com mensagens de teste |
| 17/11 | Interface recebendo e exibindo dados reais das partes 1, 2 e 3 |
| 24/11 | Refino do UX/UI, gráficos, log dos dados |
| 01/12 | Integração final + documentação |

**Depende de:** todos (é quem recebe e envia os dados de todas as partes). Responsável por manter o **protocolo Serial** documentado.

---

## Lista geral de compras/componentes

| Item | Qtd | Parte |
|---|---|---|
| Arduino Uno (ou Mega) | 1 | Todas |
| Protoboard + jumpers | — | Todas |
| Fonte 5 V externa (≥ 2 A) | 1 | 1, 3 |
| Motor de passo 28BYJ-48 + ULN2003 | 1 | 1 |
| Sensor de fim de curso / hall + ímã | 1 | 1 |
| Rolamento/base giratória | 1 | 1 |
| LED alto brilho ou módulo laser KY-008 | 1 | 2 |
| LDR | 3 | 2 (1), 3 (2) |
| Buzzer | 1 | 2 |
| Servo SG90/MG90S | 1–2 | 3 |
| Mini painel solar 5–6 V | 1–2 | 3 |
| Capacitor 470 µF | 1 | 3 |
| Resistores 10 kΩ | ~6 | 2, 3 |
| Resistor 220 Ω (LED) | 1 | 2 |
| Módulo joystick KY-023 | 1 | 4 |
| LCD 16×2 I2C (opcional) | 1 | 4 |
| INA219 (opcional) | 1 | 3 |
| MDF / impressão 3D para o corpo do satélite | — | Todos |

---

## Estrutura sugerida do repositório

```
satelite/
├── README.md              # descrição, diagrama do circuito, como rodar
├── docs/
│   ├── diagrama.png       # circuito completo (Fritzing/Tinkercad/Wokwi)
│   └── protocolo.md       # mensagens Serial
├── arduino/satelite/
│   ├── satelite.ino       # setup/loop: só chama os módulos
│   ├── motor.h / motor.cpp      # Parte 1 — Lucas
│   ├── morse.h / morse.cpp      # Parte 2 — Gabriel
│   ├── solar.h / solar.cpp      # Parte 3 — Luca
│   └── joystick.h / joystick.cpp # Parte 4 — Rafael
└── interface/
    └── app.py             # Parte 4 — Rafael
```

## Checklist da entrega (01/12)

- [ ] Todas as partes rodando juntas num único código
- [ ] Montagem física apresentável do satélite
- [ ] README com descrição, diagrama do circuito e instruções
- [ ] Vídeo de até 5 min (ideia → desenvolvimento → funcionamento), com destaque para uma coisa por vez em cena e áudio limpo
- [ ] Metas de todas as semanas atualizadas no Trello
