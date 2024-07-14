// Definindo os pinos dos LEDs
const int led1Pin = 26;
const int led2Pin = 27;
const int led3Pin = 32;
const int led4Pin = 33;

void setup() {
  // Inicializando os pinos dos LEDs como saída
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  pinMode(led4Pin, OUTPUT);

  // Inicializando a comunicação serial
  Serial.begin(115200);
}

void loop() {
  // Acende os LEDs
  digitalWrite(led1Pin, HIGH);
  digitalWrite(led2Pin, HIGH);
  digitalWrite(led3Pin, HIGH);
  digitalWrite(led4Pin, HIGH);

  Serial.println("LEDs acesos");

  // Espera 5 segundos
  delay(1000);

  // Apaga os LEDs
  digitalWrite(led1Pin, LOW);
  digitalWrite(led2Pin, LOW);
  digitalWrite(led3Pin, LOW);
  digitalWrite(led4Pin, LOW);

  Serial.println("LEDs apagados");

  // Espera mais 5 segundos antes de repetir o ciclo
  delay(1000);
}
