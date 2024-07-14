#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <Adafruit_Fingerprint.h>
#include <IRremote.h>

#define IR_RECEIVER_PIN 14
#define IR_SENDER_PIN 12
#define FINGERPRINT_SENSOR_RX 16
#define FINGERPRINT_SENSOR_TX 17

const char* ssid = "FallsBebidas";
const char* password = "87028302";

HardwareSerial mySerial(2); // UART2
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
IRrecv irReceiver(IR_RECEIVER_PIN);
IRsend irSender(IR_SENDER_PIN);
WebServer server(80);

decode_results results;
unsigned int infraredCommand = 0;

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
    html += "<tr>";
    html += "<td>Usuário 1</td>";
    html += "<td>123456789</td>";
    html += "</tr>";
    html += "<tr>";
    html += "<td>Usuário 2</td>";
    html += "<td>987654321</td>";
    html += "</tr>";
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

    // Habilitar receptor de IR
    irReceiver.enableIRIn();
    Serial.println("Receptor de infravermelho habilitado!");
}

void loop() {
  server.handleClient();
  
  // Verificar se há sinal infravermelho recebido
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

          irSender.sendNEC(command, 32);  // Corrigir aqui
          delay(5000); // Aguardar 5 segundos
          irSender.sendNEC(command, 32); // Disparar novamente o comando de infravermelho
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
    String userFilename = "/user_" + String(fingerprint) + ".txt";
    return SPIFFS.exists(userFilename);
}

bool readCommandFromSPIFFS(int fingerprint, unsigned int &command) {
    String commandFilename = "/cmd_" + String(fingerprint) + ".txt";
    if (SPIFFS.exists(commandFilename)) {
        File commandFile = SPIFFS.open(commandFilename, "r");
        if (commandFile) {
            command = commandFile.parseInt();
            commandFile.close();
            return true;
        }
    }
    return false;
}

void saveDataToSPIFFS(int fingerprint, int command) {
    String userFilename = "/user_" + String(fingerprint) + ".txt";
    File userFile = SPIFFS.open(userFilename, "w");
    if (userFile) {
        userFile.println(command);
        userFile.close();
    }
    String commandFilename = "/cmd_" + String(fingerprint) + ".txt";
    File commandFile = SPIFFS.open(commandFilename, "w");
    if (commandFile) {
        commandFile.println(command);
        commandFile.close();
    }
}
