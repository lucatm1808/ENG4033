#include <Servo.h>

int pinoServoEsquerdo = 9;
int pinoServoDireito = 10;
// Descobrir limites do painel e trocar estes valores.
int anguloMinimo = 0;
int anguloMaximo = 180;
//
unsigned long intervaloEntrePassos = 15;
unsigned long instanteAnterior = 0;
Servo servoEsquerdo;
Servo servoDireito;
int anguloAtual = 90;
int anguloAlvo = 90;

void moverParaAngulo(int angulo) {
  servoEsquerdo.write(angulo);
  servoDireito.write(180 - angulo);
}

void lerComandoSerial() {
  if (Serial.available() > 0) {
    String texto = Serial.readStringUntil('\n');
    texto.trim();
    int anguloRecebido = texto.toInt();
    // Confere se o texto era mesmo um número: converte o número de volta para texto e compara com o que foi digitado.
    // Conselho do Claude
    bool testaNumero = (texto == String(anguloRecebido));
    if (testaNumero == true && anguloRecebido >= anguloMinimo && anguloRecebido <= anguloMaximo) {
      anguloAlvo = anguloRecebido;
      Serial.println("Alvo: " + String(anguloAlvo));
    }
    else {
      Serial.println("Valor invalido: " + texto);
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
  }
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(10);
  servoEsquerdo.attach(pinoServoEsquerdo);
  servoDireito.attach(pinoServoDireito);
  moverParaAngulo(anguloAtual);
  Serial.println("Digite um angulo entre 0 e 180");
}

void loop() {
  lerComandoSerial();
  atualizarServos();
}

// IDEIAS DE REFINO (Meta 4)
// 1) Calibração: se, com o mesmo ângulo, os dois painéis não ficarem
//    alinhados (o braço de cada servo encaixa um pouco diferente),
//    criar uma correção fixa para o servo direito:
//      int correcaoServoDireito = 0;  // ajustar no teste, ex.: 5 ou -3
//      servoDireito.write(180 - angulo + correcaoServoDireito);
// 2) Controle independente (opcional): permitir comandos "E30" e "D150"
//    para mover cada painel separado, usando startsWith() e substring(). 
//    Justificativa: se um painel travar, o outro continua
//    controlável. Em satélites reais, cada asa tem o seu próprio motor,
//    mas na operação normal as duas recebem o mesmo ângulo.