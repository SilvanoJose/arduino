#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <LoRa.h>

const char* ssid = "FallsBebidas";
const char* password = "87028302";
const int loraFrequency = 915E6; // Frequência LoRa
const int ssd1306Address = 0x3c;
SSD1306Wire display(0x3c, 4, 15);
WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Configurar display SSD1306
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  // Conectar ao WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");

  // Iniciar servidor web
  server.on("/", HTTP_GET, [](){
    String tempAndHumidity = "Temperatura: XX°C, Umidade: XX%";
    // Atualizar com os dados recebidos via LoRa ou Bluetooth
    display.drawString(0, 0, tempAndHumidity);
    display.display();
    server.send(200, "text/plain", tempAndHumidity);
  });
  server.begin();
  
  // Configurar LoRa
  if (!LoRa.begin(loraFrequency)) {
    Serial.println("Erro ao iniciar LoRa");
    while (1);
  }
}

void loop() {
  server.handleClient();
  // Receber dados via LoRa ou Bluetooth e atualizar o display
}
