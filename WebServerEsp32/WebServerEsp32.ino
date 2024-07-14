#include <WiFi.h>
#include <WebServer.h>
#include <LoRa.h>
#include <HTTPClient.h>

// Informações da rede Wi-Fi
const char* ssid = "SJB";
const char* password = "56020399";

// Informações LoRa
#define LORA_SS_PIN 18  // Slave Select (SS)
#define LORA_RST_PIN 14  // Reset
#define LORA_DI0_PIN 26  // Digital Input/Output 0

// Configuração para previsão do tempo (OpenWeatherMap)
const char* weatherApiUrl = "http://api.openweathermap.org/data/2.5/weather?q=SUA_CIDADE&appid=SUA_CHAVE_API";

WebServer server(80);

void setup() {
  Serial.begin(115200);

  // Inicialização LoRa
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DI0_PIN);
  if (!LoRa.begin(433E6)) {
    Serial.println("Erro ao iniciar o módulo LoRa");
    while (1);
  }

  // Conexão Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao Wi-Fi");

  server.on("/temperature", HTTP_GET, handleTemperature);
  server.begin();
}

void loop() {
  server.handleClient();
}

void handleTemperature() {
  HTTPClient http;

  Serial.println("Obtendo dados de temperatura do OpenWeatherMap...");

  http.begin(weatherApiUrl);

  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);

      // Extrair a temperatura dos dados JSON e enviar para o cliente
      // Você precisará implementar a extração dos dados JSON a partir da resposta do OpenWeatherMap.
      // O exemplo a seguir é apenas uma simplificação.
      String temperature = "Temperatura: XX°C"; // Substitua XX pela temperatura real.

      server.send(200, "text/plain", temperature);
    } else {
      server.send(500, "text/plain", "Erro ao obter dados de temperatura");
    }
  } else {
    server.send(500, "text/plain", "Falha na conexão com o OpenWeatherMap");
  }

  http.end();
}
