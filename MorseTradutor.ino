int led = 13;

unsigned long instanteAnteriorDeDeteccao = 0;

const unsigned long pto = 200;    // 200 ms
const unsigned long traco = 600;  // 600 ms
unsigned long tempo_espera = 0;
bool estado = false;
bool percorre = false;

char caracteres[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

String codigo[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.",
  "....", "..", ".---", "-.-", ".-..", "--", "-.",
  "---", ".--.", "--.-", ".-.", "...", "-", "..-",
  "...-", ".--", "-..-", "-.--", "--..",
  "-----", ".----", "..---", "...--", "....-",
  ".....", "-....", "--...", "---..", "----."
};

//String atual[5] = { ".....", "-----", "..", "", "" };
const int TAMANHO_MAX = 25;
String atual[TAMANHO_MAX];

int indiceLetra;
int indiceCaracter;


int busca(char caracter) {
  for (int i = 0; i < 36; i++) {
    if (caracter == caracteres[i]) {
      return i;
    }
  }
  return -1;
}

void LEDMorseCode(String texto) {
  static unsigned int indice = 0;
  static unsigned long instanteAnterior = 0;
  for (int i = 0;i<TAMANHO_MAX;i++) {
    atual[i] = "";
  }

  for (int i = 0;i<texto.length();i++) {
    indice = busca(texto.charAt(i));
    atual[i] = codigo[indice];
    Serial.println(codigo[indice]);
  }

}

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
}

void loop() {
  if (millis() - instanteAnteriorDeDeteccao >= tempo_espera && !estado && percorre) {
    if (indiceCaracter < atual[indiceLetra].length()) {
      percorre = true;
      if (atual[indiceLetra][indiceCaracter] == '.') {
        estado = true;
        digitalWrite(led, HIGH);
        tempo_espera = pto;
        indiceCaracter++;
      } else if (atual[indiceLetra][indiceCaracter] == '-') {
        digitalWrite(led, HIGH);
        estado = true;
        tempo_espera = traco;
        indiceCaracter++;
      }
      instanteAnteriorDeDeteccao = millis();
    }
  }
  if (millis() - instanteAnteriorDeDeteccao >= tempo_espera && estado && percorre) {
    estado = false;
    digitalWrite(led, LOW);
    tempo_espera = pto;
    instanteAnteriorDeDeteccao = millis();
    if (indiceCaracter == atual[indiceLetra].length()) {
        indiceLetra++;
        indiceCaracter = 0;
        tempo_espera = 3*pto;
        if (atual[indiceLetra] == "") {
          percorre = false;
        }
      }
  }

  if (Serial.available() > 0) {
    String texto = Serial.readStringUntil('\n');
    texto.trim();
    LEDMorseCode(texto);
    percorre = true;
    indiceCaracter = 0;
    indiceLetra = 0;
  }
  
}
