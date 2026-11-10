int sensor = A0;

const unsigned long pto = 200;    // 200 ms
const unsigned long traco = 600;  // 600 ms
unsigned long instanteAnterior = 0;
unsigned long ultimosinal = 0;
int estado = 0;
String codigo = "";
char caracteres[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
String morses[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.",
  "....", "..", ".---", "-.-", ".-..", "--", "-.",
  "---", ".--.", "--.-", ".-.", "...", "-", "..-",
  "...-", ".--", "-..-", "-.--", "--..",
  "-----", ".----", "..---", "...--", "....-",
  ".....", "-....", "--...", "---..", "----."
};

void setup() {
  pinMode(sensor, INPUT);
  Serial.begin(9600);
}

void loop() {
  if (millis() - instanteAnterior >= 10) {  //DETECÇÂO
    int leitura = digitalRead(sensor);
    if (leitura != estado) {
      unsigned long duracao = millis() - ultimosinal;
      ultimosinal = millis();
      if (estado) {
        if (duracao < 400) {
          codigo += ".";
        } 
        else if (duracao > 400) {
          codigo += "-";
        }
      }
      estado = leitura;
    }
    if (!estado && millis() - ultimosinal >= 400 && codigo.length() > 0) { 
        for (int i = 0;i<36;i++) {
          if (codigo == morses[i]) {
            Serial.println(codigo); 
            Serial.println(caracteres[i]);
          }
        }
        codigo = "";

      }
    instanteAnterior = millis();
  }
}
