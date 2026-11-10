#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <RotaryEncoder.h>
#include <math.h>
#include <GFButton.h>

MCUFRIEND_kbv tela;
RotaryEncoder encoder(20, 21);
GFButton botao(A8);

int pot_motor = A10;
int ultimo_angulo_pot = -1;
int ultima_pos_encoder = -999;
unsigned long instante_anterior = 0;
int energia_anterior = 0;
int energia_max=0;
int angulo_max=0;
bool modo = false; //false é manual, true é automatico
int x = 0;

void tickDoEncoder() {
  encoder.tick();
}

void mudar_modo (){
  if (modo==false)
  {
    tela.fillRect(42, 135, 70, 8, TFT_BLACK);
    tela.setCursor(42, 135);
    tela.setTextColor(TFT_WHITE);
    tela.setTextSize(1);
    tela.print("Manual");
    modo = true;
  }
  else
  {
    tela.fillRect(42, 135, 70, 8, TFT_BLACK);
    tela.setCursor(42, 135);
    tela.setTextColor(TFT_WHITE);
    tela.setTextSize(1);
    tela.print("Automatico");
    modo = false;
  }
}
void setup() {
  Serial.begin(9600);
  int ID = tela.readID();
  tela.begin(ID);
  tela.setRotation(1);
  tela.fillScreen(TFT_BLACK);

  botao.setPressHandler(mudar_modo);

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

  tela.setCursor(10, 135);
  tela.setTextColor(TFT_CYAN);
  tela.setTextSize(1);
  tela.print("Modo: ");

  tela.setCursor(10, 145);
  tela.setTextColor(TFT_CYAN);
  tela.setTextSize(1);
  tela.print("Yaw: ");

  tela.setCursor(10, 155);
  tela.setTextColor(TFT_CYAN);
  tela.setTextSize(1);
  tela.print("Ang paineis: ");

  tela.setCursor(165, 138);
  tela.setTextColor(TFT_CYAN);
  tela.setTextSize(1);
  tela.print("Energia captada: ");

  tela.setCursor(165, 148);
  tela.setTextColor(TFT_CYAN);
  tela.setTextSize(1);
  tela.print("Energia maxima: ");

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
  botao.process();
  int val_pot = analogRead(pot_motor);
  int angulo_pot = map(val_pot, 0, 1023, 0, 180);
  int encoder_pos = encoder.getPosition();
  unsigned long instante_atual = millis();

  if (instante_atual > instante_anterior + 1000) {
    int energia_atual = 0;
    if (Serial.available() > 0) {
      String texto = Serial.readStringUntil('\n');
      texto.trim();
      energia_atual = texto.toInt();
    } else {
      energia_atual = random(5, 80);
    }

    Serial.print(angulo_pot);
    Serial.print(",");
    Serial.print(encoder_pos);
    Serial.print(",");
    Serial.println(energia_atual);

    tela.drawLine(166 + x, 130 - energia_anterior, 166 + x + 2, 130 - energia_atual, TFT_YELLOW);
    x += 2;
    tela.fillRect(265, 138, 50, 8, TFT_BLACK);
    tela.setCursor(265, 138);
    tela.setTextColor(TFT_WHITE);
    tela.setTextSize(1);
    tela.print(energia_atual);
    tela.print(" mW");
    if (x > 150) {
      x = 0;
      tela.fillRect(166, 37, 153, 93, TFT_BLACK);
    }
    energia_anterior = energia_atual;
    if (energia_atual>energia_max){
      energia_max = energia_atual;
      angulo_max = ultimo_angulo_pot;
      tela.fillRect(256, 148, 70, 8, TFT_BLACK);
      tela.setCursor(256, 148);
      tela.setTextColor(TFT_WHITE);
      tela.setTextSize(1);
      tela.print(energia_max);
      tela.print(" mW");

    }
    instante_anterior = instante_atual;
  }

  if (encoder_pos != ultima_pos_encoder) {
    tela.fillRect(40, 145, 70, 8, TFT_BLACK);
    tela.setCursor(40, 145);
    tela.setTextColor(TFT_WHITE);
    tela.setTextSize(1);
    tela.print(encoder_pos);
    tela.print(" deg");
    ultima_pos_encoder = encoder_pos;
  }

  if (modo){
    if (angulo_pot != ultimo_angulo_pot) {
      tela.fillRect(85, 155, 70, 8, TFT_BLACK);
      tela.setCursor(85, 155);
      tela.setTextColor(TFT_WHITE);
      tela.setTextSize(1);
      tela.print(angulo_pot);
      tela.print(" deg");
      ultimo_angulo_pot = angulo_pot;

      int cx = 80;
      int cy = 83;
      int raio = 42;
      float rad = angulo_pot * PI/180.0;
      int x1 = cx - (raio * cos(rad)); //cos(rad) calcula a projeção horizontal
      int y1 = cy + (raio * sin(rad)); //sin(rad) calcula a projeção vertical
      int x2 = cx + (raio * cos(rad)); 
      int y2 = cy - (raio * sin(rad));
      tela.fillRect(6, 37, 143, 93, TFT_BLACK);
      tela.drawLine(x1, y1, x2, y2, TFT_CYAN);
      tela.drawLine(x1, y1 + 1, x2, y2 + 1, TFT_CYAN);
      tela.fillCircle(cx, cy, 3, TFT_WHITE);
    }
  }
}
