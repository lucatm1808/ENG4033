#include <Servo.h>

int pinoServoEsquerdo = 9;
int pinoServoDireito = 10;
int anguloMinimo = 0;
int anguloMaximo = 180;
unsigned long intervaloEntrePassos = 30;
unsigned long instanteAnterior = 0;
Servo servoEsquerdo;
Servo servoDireito;
int anguloAtual = 90;
int anguloAlvo = 90;
//0 = parada (controle manual)
//1 = indo para o 0 (posição inicial da varredura)
//2 = indo do 0 ao 180
int etapaVarredura = 0;
int pinoPlacaSolar = A0;
int tensaoMaximaMv = 5000;
int maiorTensao = 0;
int melhorAngulo = 90;

int numeroDeEnergias = 20;
unsigned long intervaloEntreEnergias = 1;
unsigned long instanteUltimaEnergia = 0;
unsigned long somaEnergias = 0;
int energiasLidas = 0;
int tensaoMedia = 0;

void moverParaAngulo(int angulo) {
  servoEsquerdo.write(angulo);
  servoDireito.write(180 - angulo);
}

void lerComandoSerial() {
  if (Serial.available() > 0) {
    String texto = Serial.readStringUntil('\n');
    texto.trim();
    if (texto == "energia") {
      Serial.println("Tensao: " + String(tensaoMedia) + " mV");
    }
    else if (texto == "varrer") {
      iniciarVarredura();
    }
    else {
      anguloAlvo = texto.toInt();
      etapaVarredura = 0;
      Serial.println("Alvo: " + String(anguloAlvo));
    }
  }
}

void atualizarServos() {
  if (anguloAtual != anguloAlvo && millis() - instanteAnterior > intervaloEntrePassos) {
    instanteAnterior = millis();
    if (anguloAlvo > anguloAtual) {
      anguloAtual = anguloAtual + 1;
    }
    else {
      anguloAtual = anguloAtual - 1;
    }
    moverParaAngulo(anguloAtual);
    
    if (anguloAtual == anguloAlvo) {
      Serial.println("Chegou em: " + String(anguloAtual));
    }

    if (etapaVarredura == 2) {
      if (tensaoMedia > maiorTensao) {
        maiorTensao = tensaoMedia;
        melhorAngulo = anguloAtual;
      }
    }
  }
}

void lerTensaoPlaca() {
  if (millis() - instanteUltimaEnergia >= intervaloEntreEnergias) {
    instanteUltimaEnergia = millis();
    somaEnergias = somaEnergias + analogRead(pinoPlacaSolar);
    energiasLidas = energiasLidas + 1;

    if (energiasLidas == numeroDeEnergias) {
      int leituraMedia = somaEnergias / numeroDeEnergias;
      tensaoMedia = map(leituraMedia, 0, 1023, 0, tensaoMaximaMv);
      somaEnergias = 0;
      energiasLidas = 0;
    }
  }
}

void iniciarVarredura() {
  anguloAlvo = anguloMinimo;
  etapaVarredura = 1;
  Serial.println("Iniciando varredura");
}

void atualizarVarredura() {
  if (etapaVarredura == 1 && anguloAtual == anguloMinimo) {
    maiorTensao = tensaoMedia;
    melhorAngulo = anguloMinimo;
    anguloAlvo = anguloMaximo;
    etapaVarredura = 2;
  }
  if (etapaVarredura == 2 && anguloAtual == anguloMaximo) {
    Serial.println("Melhor angulo: " + String(melhorAngulo) + " (" + String(maiorTensao) + " mV)");
    anguloAlvo = melhorAngulo;
    etapaVarredura = 0;
  }
}


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(10);
  servoEsquerdo.attach(pinoServoEsquerdo);
  servoDireito.attach(pinoServoDireito);
  pinMode(pinoPlacaSolar, INPUT);
  moverParaAngulo(anguloAtual);
  Serial.println("Comandos: angulo (0 a 180), energia, varrer");
}


void loop() {
  lerTensaoPlaca();
  lerComandoSerial();
  atualizarServos();
  atualizarVarredura();
}
