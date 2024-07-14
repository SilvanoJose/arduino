#include <Wire.h>
#include <SSD1306Wire.h>
#include <LoRa.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Alunos";
const char* password = "";
const int loraFrequency = 915E6;
const int ssd1306Address = 0x3c;
SSD1306Wire display(0x3c, 4, 15);
WebServer server(80);
String receivedData = "Nenhum dado recebido"; // Inicialize com uma mensagem padrão

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
  server.on("/", HTTP_GET, []() {
    String temperatura = receivedData;
    display.clear();
    display.drawString(0, 0, temperatura);
    display.display();
    server.send(200, "text/plain", temperatura);
  });
  server.begin();

  // Configurar LoRa
  if (!LoRa.begin(loraFrequency)) {
    Serial.println("Erro ao iniciar LoRa");
    while (1);
  }
  LoRa.onReceive(receiveData);
  LoRa.receive();
}

void loop() {
  server.handleClient();
}

void receiveData(int packetSize) {
  if (packetSize == 0) {
    return;
  }

  while (LoRa.available()) {
    receivedData = LoRa.readString();
    display.clear();
    display.drawString(0, 0, receivedData);
  }
}
