#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "SJB";
const char* password = "56020399";

// Substitua a URL pelo serviço de weather que você está utilizando.
const char* weatherServiceURL = "1c25e81fe505234968cff6e2375491ab";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  Serial.println("Conectado ao WiFi");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    // Realiza uma solicitação HTTP ao serviço de weather
    HTTPClient http;
    http.begin(weatherServiceURL);
    int httpCode = http.GET();
    String payload = "{}"; // JSON de resposta padrão em caso de erro

    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
    }

    http.end();

    // Parse do JSON
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    float temperatura = doc["main"]["temp"];

    // Cria a resposta HTML com a temperatura
    String html = "<html><body>";
    html += "<h1>Temperatura:</h1>";
    html += "<p>" + String(temperatura) + " &deg;C</p>";
    html += "</body></html>";

    request->send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
  // Coloque aqui outras ações que você deseja realizar no loop
}
