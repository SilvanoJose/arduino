#include <SPI.h>
#include <LoRa.h>

const int lm35Pin = A0;
float temperatura = 0.0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Inicialize o sensor LM35 e a comunicação LoRa
  analogReference(DEFAULT); 
  LoRa.begin(915E6); 
 }

void loop() {
  int sensorValue = analogRead(lm35Pin);
  temperatura = (sensorValue / 1024.0) * 500.0;

  // Envie os dados via LoRa
  LoRa.beginPacket();
  LoRa.print("Temperatura: ");
  LoRa.print(temperatura);
  LoRa.print(" °C");
  LoRa.endPacket();

  Serial.print("Temperatura enviada: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  delay(10000); // Envie os dados a cada 10 segundos
}
