#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <RotaryEncoder.h>

MCUFRIEND_kbv tela;
RotaryEncoder encoder(20, 21);
int pot_motor = A10;
int ultimo_angulo_pot = -1;
int ultima_pos_encoder = -999;
unsigned long instante_anterior = 0;
int energia_anterior = 0;
int x=0;

void tickDoEncoder() {
  encoder.tick();
}

void setup() {
  Serial.begin(9600);

  int ID = tela.readID();
  tela.begin(ID);

  tela.setRotation(1);
  tela.fillScreen(TFT_BLACK);

  int origem1 = digitalPinToInterrupt(20);
  attachInterrupt(origem1, tickDoEncoder, CHANGE);
  int origem2 = digitalPinToInterrupt(21);
  attachInterrupt(origem2, tickDoEncoder, CHANGE);

  tela.fillRect(0, 0, 320, 30, TFT_NAVY);
  tela.setCursor(20, 7);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(2);
  tela.print("Ground Station Satelite");

  tela.drawRect(5, 36, 155, 95, TFT_WHITE);
  tela.drawRect(165, 36, 155, 95, TFT_WHITE);

  tela.setCursor(10, 138);
  tela.setTextColor(TFT_CYAN);
  tela.setTextSize(1);
  tela.print("Yaw: ");

  tela.setCursor(10, 148);
  tela.setTextColor(TFT_CYAN);
  tela.setTextSize(1);
  tela.print("Ang paineis: ");

  tela.setCursor(165, 138);
  tela.setTextColor(TFT_CYAN);
  tela.setTextSize(1);
  tela.print("Energia captada: ");
  tela.setTextColor(TFT_WHITE);
  tela.println("Z mW");

  tela.setCursor(80, 170);
  tela.setTextColor(TFT_WHITE);
  tela.setTextSize(1);
  tela.print("Mensagem traduzida:");

  tela.drawRect(10, 182, 300, 40, TFT_WHITE);
  tela.setCursor(20, 193);
  tela.setTextColor(TFT_CYAN);
  tela.setTextSize(2);
  tela.print("EU SOU UM SATELITE");
}

void loop() {
  int val_pot = analogRead(pot_motor);
  int angulo_pot = map(val_pot, 0, 1023, 0, 180);
  int encoder_pos = encoder.getPosition();
  unsigned long instante_atual = millis();
  if (instante_atual > instante_anterior + 1000) {
  Serial.print(angulo_pot);
  Serial.print(",");
  Serial.println(encoder_pos);
  tela.drawLine(165+x, 131-energia_anterior, 165+x++, 131-energia_atual, TFT_WHITE);
  x++;
  energia_anterior = energia_atual;
  instante_anterior = instante_atual;
  }

  if (encoder_pos != ultima_pos_encoder) {
    tela.fillRect(40, 138, 70, 8, TFT_BLACK);
    tela.setCursor(40, 138);
    tela.setTextColor(TFT_WHITE);
    tela.setTextSize(1);
    tela.print(encoder_pos);
    tela.print(" deg");
    ultima_pos_encoder = encoder_pos;
  }
  if (angulo_pot != ultimo_angulo_pot) {
    tela.fillRect(85, 148, 70, 8, TFT_BLACK);
    tela.setCursor(85, 148);
    tela.setTextColor(TFT_WHITE);
    tela.setTextSize(1);
    tela.print(angulo_pot);
    tela.print(" deg");
    ultimo_angulo_pot = angulo_pot;
  }
}
