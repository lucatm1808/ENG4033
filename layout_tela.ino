#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <RotaryEncoder.h>

MCUFRIEND_kbv tela;
RotaryEncoder encoder(20, 21);

int pot_motor = A0;

int ultimo_angulo_motor = -1;
int ultima_pos_encoder = -999;

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
  tela.drawFastHLine(0, 32, 320, TFT_WHITE);

  tela.drawRect(5, 36, 155, 95, TFT_WHITE);
  tela.drawRect(165, 36, 155, 95, TFT_WHITE);

  tela.setCursor(10, 133);
  tela.setTextColor(TFT_CYAN);
  tela.setTextSize(1);
  tela.print("Yaw: ");

  tela.setCursor(10, 143);
  tela.setTextColor(TFT_CYAN);
  tela.setTextSize(1);
  tela.print("Ang paineis: ");

  tela.setCursor(165, 133);
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
  int angulo_motor = map(val_pot, 0, 1023, 0, 360);
  int encoder_pos = encoder.getPosition();

  Serial.print(angulo_motor);
  Serial.print(",");
  Serial.println(encoder_pos);

  if (angulo_motor != ultimo_angulo_motor) {
    tela.fillRect(40, 133, 70, 8, TFT_BLACK);
    tela.setCursor(40, 133);
    tela.setTextColor(TFT_WHITE);
    tela.setTextSize(1);
    tela.print(angulo_motor);
    tela.print(" deg");
    ultimo_angulo_motor = angulo_motor;
  }

  if (encoder_pos != ultima_pos_encoder) {
    tela.fillRect(85, 143, 70, 8, TFT_BLACK);
    tela.setCursor(85, 143);
    tela.setTextColor(TFT_WHITE);
    tela.setTextSize(1);
    tela.print(encoder_pos);
    tela.print(" deg");
    ultima_pos_encoder = encoder_pos;
  }

  delay(50);
}
