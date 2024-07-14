#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <Adafruit_Fingerprint.h>

#define FINGERPRINT_SENSOR_RX 16
#define FINGERPRINT_SENSOR_TX 17

const char* ssid = "FallsBebidas";
const char* password = "87028302";

HardwareSerial mySerial(2); // UART2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
WebServer server(80);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void handleRoot() {
    String html = "<html><head>";
    html += "<title>Cadastro de Usuário</title>";
    html += "<style>";
    // Estilos CSS
    html += "</style>";
    html += "<h2>Cadastro de Usuário</h2>";
    html += "<form action='/register' method='post'>";
    html += "<label for='name'>Nome:</label><br>";
    html += "<input type='text' id='name' name='name'><br>";
    html += "<button type='button' onclick='startFingerprintCapture()'>Capturar Impressão Digital</button>";
    html += "<input type='text' id='fingerprint' name='fingerprint' readonly><br>"; // Campo de texto de leitura apenas para mostrar a digital capturada
    html += "<input type='submit' value='Cadastrar'>";
    html += "</form>";
    html += "<script>";
    html += "function startFingerprintCapture() {";
    html += "document.getElementById('fingerprint').value = 'Aguardando captura...';";
    html += "var xhttp = new XMLHttpRequest();";
    html += "xhttp.onreadystatechange = function() {";
    html += "if (this.readyState == 4 && this.status == 200) {";
    html += "document.getElementById('fingerprint').value = this.responseText;";
    html += "}";
    html += "};";
    html += "xhttp.open('GET', '/startCapture', true);";
    html += "xhttp.send();";
    html += "}";
    html += "</script>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

void handleRegister() {
    String name = server.arg("name");
    String fingerprint = server.arg("fingerprint");

    // Salvar nome e impressão digital no SPIFFS
    saveDataToSPIFFS(name, fingerprint);

    server.send(200, "text/html", "Registrado com sucesso!");
}

void setup() {
    Serial.begin(115200);
    mySerial.begin(57600, SERIAL_8N1, FINGERPRINT_SENSOR_RX, FINGERPRINT_SENSOR_TX);
    finger.begin(57600);
    setup_wifi();
    
    Serial.println("Iniciando SPIFFS...");
    if (!SPIFFS.begin(true)) {
        Serial.println("Falha ao montar o sistema de arquivos SPIFFS.");
        while (true); // Trava para depuração
    }
    Serial.println("Sistema de arquivos SPIFFS montado com sucesso!");

    // Configurações do servidor web
    server.on("/", handleRoot);
    server.on("/register", HTTP_POST, handleRegister);

    server.onNotFound([]() {
        Serial.println("Requisição 404 - Página não encontrada");
        server.send(404, "text/plain", "Página não encontrada");
    });

    server.begin();
    Serial.println("Servidor HTTP iniciado!");
}

void loop() {
  server.handleClient();
}

void saveDataToSPIFFS(String name, String fingerprint) {
    Serial.print("Usuário cadastrado com sucesso - Nome: ");
    Serial.print(name);
    Serial.print(", Digital: ");
    Serial.println(fingerprint);

    File file = SPIFFS.open("/userdata.txt", "w");
    if (file) {
        file.print("Name: ");
        file.println(name);
        file.print("Fingerprint: ");
        file.println(fingerprint);
        file.close();
    }
}

