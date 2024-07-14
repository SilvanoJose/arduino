#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

SoftwareSerial mySerial(D2, D3); // Defina os pinos RX e TX nos pinos desejados (ex.: D2 e D3)
Adafruit_Fingerprint fingerprintSensor = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(115200); // Inicializa a Serial0 para comunicação com o PC
  mySerial.begin(57600); // Inicializa a SoftwareSerial

  fingerprintSensor.begin(57600);

  if (!fingerprintSensor.verifyPassword()) {
    Serial.println("Não foi possível conectar ao sensor. Verifique a senha ou a conexão");
    while (true);
  } else {
    Serial.println("Sensor conectado com sucesso!");
  }
}

void loop() {
  // Loop vazio para teste inicial
}
