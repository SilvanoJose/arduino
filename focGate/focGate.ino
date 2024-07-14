#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <Adafruit_Fingerprint.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <WebServer.h>

// Credenciais WiFi
const char* ssid = "FallsBebidas";
const char* password = "87028302";

// Definição dos sensores
#define FINGERPRINT_SENSOR_RX 16  // Verifique se esses pinos estão corretos para o seu hardware
#define FINGERPRINT_SENSOR_TX 17
#define IR_RECEIVER_PIN 4
#define IR_SENDER_PIN 5

// Instanciar classes de sensores e servidor
HardwareSerial mySerial(2); // UART2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
IRrecv irReceiver(IR_RECEIVER_PIN);
IRsend irSender(IR_SENDER_PIN);
WebServer server(80);

// Variável global
unsigned int infraredCommand = 0;

void handleRoot() {
  String html = "<html><head>";
  html += "<title>Cadastro de Usuário</title>";
  html += "<style>";
  html += "table {";
  html += "border-collapse: collapse;";
  html += "width: 100%;";
  html += "}";
  html += "th, td {";
  html += "border: 1px solid #dddddd;";
  html += "text-align: left;";
  html += "padding: 8px;";
  html += "}";
  html += "th {";
  html += "background-color: #f2f2f2;";
  html += "}";
  html += ".column {";
  html += "float: left;";
  html += "width: 50%;";
  html += "}";
  html += ".row:after {";
  html += "content: '';";
  html += "display: table;";
  html += "clear: both;";
  html += "}";
  html += "</style>";
  html += "<h2>Cadastro de Usuário</h2>";
  html += "<div class='row'>";
  html += "<div class='column'>";
  html += "<h3>Registro de Usuário</h3>";
  html += "<form action='/register' method='post'>";
  html += "<label for='name'>Nome:</label><br>";
  html += "<input type='text' id='name' name='name'><br>";
  html += "<h3>Captura de Digital</h3>";
  html += "<button type='button' onclick='startFingerprintCapture()'>Iniciar Captura</button>";
  html += "<p>Estado da captura: <span id='captureStatus'>Aguardando captura...</span></p>";
  html += "<br>";
  html += "<label for='fingerprint'>Leitura da Digital:</label><br>";
  html += "<input type='text' id='fingerprint' name='fingerprint'><br><br>";
  html += "<input type='submit' value='Cadastrar'>";
  html += "</form>";
  html += "</div>";
  html += "<div class='column'>";
  html += "<h3>Captura de Infravermelho</h3>";
  html += "<button type='button' onclick='captureInfrared()'>Iniciar Captura</button>";
  html += "<p>Código do Sinal: <span id='infraredCode'></span></p>";
  html += "</div>";
  html += "</div>";

  html += "<h2>Usuários Cadastrados</h2>";
  html += "<table>";
  html += "<tr>";
  html += "<th>Nome</th>";
  html += "<th>ID da Impressão Digital</th>";
  html += "</tr>";
  // Aqui você pode adicionar as linhas da tabela com os usuários cadastrados
  html += "<tr>";
  html += "<td>Usuário 1</td>";
  html += "<td>123456789</td>";
  html += "</tr>";
  html += "<tr>";
  html += "<td>Usuário 2</td>";
  html += "<td>987654321</td>";
  html += "</tr>";
  // Adicione mais linhas conforme necessário
  html += "</table>";

  html += "<script>";
  html += "function startFingerprintCapture() {";
  html += "document.getElementById('captureStatus').innerText = 'Capturando...';";
  html += "var xhttp = new XMLHttpRequest();";
  html += "xhttp.onreadystatechange = function() {";
  html += "if (this.readyState == 4 && this.status == 200) {";
  html += "document.getElementById('captureStatus').innerText = 'Aguardando captura...';";
  html += "}";
  html += "};";
  html += "xhttp.open('GET', '/startCapture', true);";
  html += "xhttp.send();";
  html += "}";
  html += "function captureInfrared() {";
  html += "document.getElementById('infraredCode').innerText = '123456';";
  html += "}";
  html += "</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleRegister() {
  String fingerprint = server.arg("fingerprint");
  String command = server.arg("command");

  // Salvar fingerprint e command no SPIFFS
  saveDataToSPIFFS(fingerprint.toInt(), command.toInt());

  server.send(200, "text/html", "Registrado com sucesso!");
}

void setup() {
  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, FINGERPRINT_SENSOR_RX, FINGERPRINT_SENSOR_TX);
  finger.begin(57600);
  
  Serial.println("Iniciando conexão WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

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

  // Habilitar receptor de IR
  irReceiver.enableIRIn();
  Serial.println("Receptor de infravermelho habilitado!");
}

void loop() {
  Serial.println("Dentro do loop...");
  server.handleClient();
  // Verificar se há sinal infravermelho recebido
  decode_results results;
  if (irReceiver.decode(&results)) {
    irReceiver.resume();
    infraredCommand = results.value;
    Serial.print("Comando Infravermelho Recebido: ");
    Serial.println(infraredCommand);
  }

  // Verificar se há impressão digital válida
  if (finger.getImage() == FINGERPRINT_OK) {
    int fingerprint = finger.image2Tz();
    if (fingerprint == FINGERPRINT_OK) {
      fingerprint = finger.fingerID;

      // Verificar se a impressão digital está cadastrada
      if (isUserRegistered(fingerprint)) {
        Serial.print("Usuário registrado encontrado. ID: ");
        Serial.println(fingerprint);

        // Verificar se há comando de infravermelho correspondente
        unsigned int command;
        if (readCommandFromSPIFFS(fingerprint, command)) {
          Serial.print("Enviando comando IR correspondente: ");
          Serial.println(command);

          irSender.sendNEC(command);
          delay(5000); // Aguardar 5 segundos
          irSender.sendNEC(command); // Disparar novamente o comando de infravermelho
        } else {
          Serial.println("Nenhum comando IR encontrado para essa impressão digital.");
        }
      } else {
        Serial.println("Impressão digital não registrada.");
      }
    } else {
      Serial.println("Falha ao converter a imagem do sensor.");
    }
  }
}

bool isUserRegistered(int fingerprint) {
  String filename = "/user_" + String(fingerprint) + ".txt";
  return SPIFFS.exists(filename);
}

bool readCommandFromSPIFFS(int fingerprint, unsigned int& command) {
  String filename = "/command_" + String(fingerprint) + ".txt";
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    return false;
  }
  
  String content = file.readString();
  file.close();
  command = content.toInt();
  return true;
}

void saveDataToSPIFFS(int fingerprint, unsigned int command) {
  String userFilename = "/user_" + String(fingerprint) + ".txt";
  String commandFilename = "/command_" + String(fingerprint) + ".txt";

  // Salvar fingerprint no arquivo de usuário
  File userFile = SPIFFS.open(userFilename, "w");
  if (userFile) {
    userFile.print(fingerprint);
    userFile.close();
  }

  // Salvar comando no arquivo de comando
  File commandFile = SPIFFS.open(commandFilename, "w");
  if (commandFile) {
    commandFile.print(command);
    commandFile.close();
  }
}
