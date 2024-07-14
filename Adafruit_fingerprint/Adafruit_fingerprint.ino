#include <Adafruit_Fingerprint.h>

// Utilize a porta serial do hardware do ESP32
HardwareSerial mySerial(2); // Utilize UART2 (GPIO16 para RX e GPIO17 para TX)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int getFingerprintIDez();

void setup()  {  
  Serial.begin(115200);
  Serial.println("Iniciando Leitor Biometrico");

  // Definindo os pinos GPIO 11 e 12
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  // Iniciando a comunicação serial com o sensor biométrico
  mySerial.begin(57600, SERIAL_8N1, 16, 17); // RX=16, TX=17

  delay(1000); // Aguarde um pouco para o sensor iniciar
  
  Serial.println("Verificando leitor biométrico...");
  if (finger.verifyPassword()) {
    Serial.println("Leitor Biometrico Encontrado");
  } else {
    Serial.println("Leitor Biometrico nao encontrado");
    while (1) {
      // Mantenha piscando um LED para indicar que está esperando
      digitalWrite(12, HIGH);
      delay(250);
      digitalWrite(12, LOW);
      delay(250);
    }
  }
  Serial.println("Esperando Dedo para Verificar");
}

void loop()                   
{
  getFingerprintIDez();
  digitalWrite(12, HIGH);
  delay(50);          
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagem Capturada");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("Dedo nao Localizado");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro ao se comunicar");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Erro ao Capturar");
      return p;
    default:
      Serial.println("Erro desconhecido");
      return p;
  }

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagem Convertida");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Imagem muito confusa");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Erro ao se comunicar");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Impossivel localizar Digital");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Impossivel Localizar Digital");
      return p;
    default:
      Serial.println("Erro Desconhecido");
      return p;
  }
  
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Digital Encontrada");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Erro ao se comunicar");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Digital Desconhecida");
    return p;
  } else {
    Serial.println("Erro Desconhecido");
    return p;
  }   
  
  Serial.print("ID # Encontrado"); 
  Serial.print(finger.fingerID); 
  Serial.print(" com precisao de "); 
  Serial.println(finger.confidence); 
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  digitalWrite(12, LOW);
  digitalWrite(11, HIGH);
  delay(1000);
  digitalWrite(11, LOW);
  delay(1000);
  digitalWrite(12, HIGH);
  Serial.print("ID # Encontrado"); 
  Serial.print(finger.fingerID); 
  Serial.print(" com precisao de "); 
  Serial.println(finger.confidence);
  return finger.fingerID; 
}
