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
int ultimoModo = 0;
unsigned long intervaloEntreEnvios = 200;
unsigned long instanteUltimoEnvio = 0;
int pinoLed = 13;

// Refino: 
// Correção fixa, em graus, somada ao servo direito.
// Serve para alinhar os dois painéis se, com o mesmo ângulo, eles
// ficarem um pouco diferentes (o braço de cada servo encaixa numa
// posição diferente). Ajustar no teste, ex.: 5 ou -3. Com 0 não muda nada.
int correcaoServoDireito = 0;

void moverParaAngulo(int angulo) {
  servoEsquerdo.write(angulo);
  servoDireito.write(180 - angulo + correcaoServoDireito);
}

void lerComandoSerial1() {
  if (Serial1.available() > 0) {
    String texto = Serial1.readStringUntil('\n');
    texto.trim();
    int modoRecebido = texto.substring(0, 1).toInt();
    int estadoLedRecebido = texto.substring(2, 3).toInt();
    int anguloRecebido = texto.substring(4).toInt();
    // 0 = manual, 1 = automático.
    if (modoRecebido == 0) {
      anguloAlvo = anguloRecebido;
      etapaVarredura = 0;
    }
    else if (ultimoModo == 0) {
      iniciarVarredura();
    }
    ultimoModo = modoRecebido;
    if (estadoLedRecebido == 1) {
      digitalWrite(pinoLed, HIGH);
    }
    else {
      digitalWrite(pinoLed, LOW);
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
}

void atualizarVarredura() {
  if (etapaVarredura == 1 && anguloAtual == anguloMinimo) {
    maiorTensao = tensaoMedia;
    melhorAngulo = anguloMinimo;
    anguloAlvo = anguloMaximo;
    etapaVarredura = 2;
  }
  if (etapaVarredura == 2 && anguloAtual == anguloMaximo) {
    anguloAlvo = melhorAngulo;
    etapaVarredura = 0;
  }
}

void enviarDados() {
  if (millis() - instanteUltimoEnvio >= intervaloEntreEnvios) {
    instanteUltimoEnvio = millis();
    Serial1.println(String(anguloAtual) + ";" + String(tensaoMedia));
  }
}

void setup() {
  Serial1.begin(9600);
  //Serial.begin(9600);
  Serial1.setTimeout(10);
  servoEsquerdo.attach(pinoServoEsquerdo);
  servoDireito.attach(pinoServoDireito);
  pinMode(pinoPlacaSolar, INPUT);
  pinMode(pinoLed, OUTPUT);
  moverParaAngulo(anguloAtual);
}

void loop() {
  lerTensaoPlaca();
  lerComandoSerial1();
  atualizarServos();
  atualizarVarredura();
  enviarDados();
}
