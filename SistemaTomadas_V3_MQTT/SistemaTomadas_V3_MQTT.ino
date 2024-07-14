#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>

const char* mqtt_server = "endereco_do_broker";  // Substitua pelo endereço do seu broker MQTT
const char* mqtt_client_id = "esp32-client";
const char* mqtt_topic1 = "/tomada1";
const char* mqtt_topic2 = "/tomada2";
const char* mqtt_topic3 = "/tomada3";
const char* mqtt_topic4 = "/tomada4";

const char* ssid = "FallsBebidas";
const char* password = "87028302";
IPAddress localIP(192, 168, 0, 198); // Endereço IP fixo
IPAddress gateway(192, 168, 0, 1);   // Endereço IP do gateway
IPAddress subnet(255, 255, 255, 0);  // Máscara de sub-rede

WebServer server(80);

WiFiClient espClient;
PubSubClient client(espClient);


const int ledPin1 = 27;
const int ledPin2 = 26;
const int ledPin3 = 32;
const int ledPin4 = 33;
bool ledStatus1 = false;
bool ledStatus2 = false;
bool ledStatus3 = false;
bool ledStatus4 = false;

void setup() {
  Serial.begin(115200);

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, LOW);
  digitalWrite(ledPin4, LOW);


  if (!SPIFFS.begin(true)) {
    Serial.println("Ocorreu um erro ao montar o SPIFFS");
    return;
  }

File file1 = SPIFFS.open("/led_state1.txt", "r");
  if (file1) {
    ledStatus1 = file1.readStringUntil('\n').toInt();
    if (ledStatus1 == 1){
      digitalWrite(ledPin1, HIGH);
    }
    file1.close();
  }

File file2 = SPIFFS.open("/led_state2.txt", "r");
  if (file2) {
    ledStatus2 = file2.readStringUntil('\n').toInt();
    if (ledStatus2 == 1){
      digitalWrite(ledPin2, HIGH);
    }
    file2.close();
  }

File file3 = SPIFFS.open("/led_state3.txt", "r");
  if (file3) {
    ledStatus3 = file3.readStringUntil('\n').toInt();
    if (ledStatus3 == 1){
      digitalWrite(ledPin3, HIGH);
    }
    file3.close();
  }

  File file4 = SPIFFS.open("/led_state4.txt", "r");
  if (file4) {
    ledStatus4 = file4.readStringUntil('\n').toInt();
    if (ledStatus4 == 1){
      digitalWrite(ledPin4, HIGH);
    }
    file4.close();
  }



  // Conectar-se à rede Wi-Fi
  WiFi.config(localIP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando-se à rede Wi-Fi...");
  }

  // Imprimir o endereço IP no monitor serial
  Serial.println("");
  Serial.print("Conectado à rede Wi-Fi ");
  Serial.print(ssid);
  Serial.print(" | IP: ");
  Serial.println(WiFi.localIP());

  // Conectar-se ao broker MQTT
  client.setServer(mqtt_server, 1883);  // Substitua 1883 pela porta do seu broker MQTT
  client.setCallback(callback);
  reconnect();


  server.on("/", handleRoot);
  server.on("/ligar1", handleLigar1);
  server.on("/desligar1", handleDesligar1);
  server.on("/ligar2", handleLigar2);
  server.on("/desligar2", handleDesligar2);
  server.on("/ligar3", handleLigar3);
  server.on("/desligar3", handleDesligar3);
  server.on("/ligar4", handleLigar4);
  server.on("/desligar4", handleDesligar4);

  server.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  server.handleClient();
}



void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("conectado!");
      // Inscreva-se nos tópicos MQTT
      client.subscribe(mqtt_topic1);
      client.subscribe(mqtt_topic2);
      client.subscribe(mqtt_topic3);
      client.subscribe(mqtt_topic4);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando reconectar em 5 segundos...");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  // Processar a mensagem e alterar os pinos conforme necessário
  if (strcmp(topic, mqtt_topic1) == 0) {
    int value = (char)payload[0] - '0';
    digitalWrite(ledPin1, value);
    ledStatus1 = value;
    writeLedStateToFile(ledStatus1, "/led_state1.txt");
    
  } else if (strcmp(topic, mqtt_topic2) == 0) {
    int value = (char)payload[0] - '0';
    digitalWrite(ledPin2, value);
    ledStatus1 = value;
    writeLedStateToFile(ledStatus2, "/led_state2.txt");
  }

   else if (strcmp(topic, mqtt_topic3) == 0) {
    int value = (char)payload[0] - '0';
    digitalWrite(ledPin3, value);
    ledStatus1 = value;
    writeLedStateToFile(ledStatus3, "/led_state3.txt");
  }

  else if (strcmp(topic, mqtt_topic4) == 0) {
    int value = (char)payload[0] - '0';
    digitalWrite(ledPin4, value);
    ledStatus1 = value;
    writeLedStateToFile(ledStatus4, "/led_state4.txt");
  }
}


void handleRoot() {
  String html = "<html><head>";
  html += "<meta charset='UTF-8'>"; // Adicione esta linha para definir a codificação
  html += "<style>";
  html += "body {text-align: center;}";
  html += ".btn {padding: 10px 20px; font-size: 20px;}";
  html += ".popup {display: none; position: fixed; top: 50%; left: 50%; transform: translate(-50%, -50%); background: white; padding: 20px; border: 1px solid #ccc;}";
  html += ".overlay {display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0, 0, 0, 0.5);}";
  html += "</style></head><body>";
  
  html += "<h1>Controle de Tomadas</h1>";
  
  html += "<p>Tomada 1 - Status: ";
  html += ledStatus1 ? "Ligado" : "Desligado";
  html += "</p>";

  html += ledStatus1 ? "<button class='btn' disabled>Ligar Tomada 1</button>" : "<button class='btn' onclick='showPopup(\"ligar1\")'>Ligar Tomada 1</button>";
  html += ledStatus1 ? "<button class='btn' onclick='showPopup(\"desligar1\")'>Desligar Tomada 1</button>" : "<button class='btn' disabled>Desligar Tomada 1</button>";

  
  html += "<p>Tomada 2 - Status: ";
  html += ledStatus2 ? "Ligado" : "Desligado";
  html += "</p>";

  html += ledStatus2 ? "<button class='btn' disabled>Ligar Tomada 2</button>" : "<button class='btn' onclick='showPopup(\"ligar2\")'>Ligar Tomada 2</button>";
  html += ledStatus2 ? "<button class='btn' onclick='showPopup(\"desligar2\")'>Desligar Tomada 2</button>" : "<button class='btn' disabled>Desligar Tomada 2</button>";


  html += "<p>Tomada 3 - Status: ";
  html += ledStatus3 ? "Ligado" : "Desligado";
  html += "</p>";

  html += ledStatus3 ? "<button class='btn' disabled>Ligar Tomada 3</button>" : "<button class='btn' onclick='showPopup(\"ligar3\")'>Ligar Tomada 3</button>";
  html += ledStatus3 ? "<button class='btn' onclick='showPopup(\"desligar3\")'>Desligar Tomada 3</button>" : "<button class='btn' disabled>Desligar Tomada 3</button>";


  html += "<p>Tomada 4 - Status: ";
  html += ledStatus4 ? "Ligado" : "Desligado";
  html += "</p>";

  html += ledStatus4 ? "<button class='btn' disabled>Ligar Tomada 4</button>" : "<button class='btn' onclick='showPopup(\"ligar4\")'>Ligar Tomada 4</button>";
  html += ledStatus4 ? "<button class='btn' onclick='showPopup(\"desligar4\")'>Desligar Tomada 4</button>" : "<button class='btn' disabled>Desligar Tomada 4</button>";

 
  html += "<div class='overlay' id='overlay' onclick='hidePopup()'></div>";
  html += "<div class='popup' id='popup'>";
  html += "<p>Deseja confirmar a ação?</p>";
  html += "<button class='btn' onclick='confirmAction()'>Confirmar</button>";
  html += "<button class='btn' onclick='hidePopup()'>Cancelar</button>";
  html += "</div>";

  html += "<script>";
  html += "function showPopup(action) {";
  html += "  document.getElementById('overlay').style.display = 'block';";
  html += "  document.getElementById('popup').style.display = 'block';";
  html += "  document.getElementById('popup').setAttribute('data-action', action);";
  html += "}";
  html += "function hidePopup() {";
  html += "  document.getElementById('overlay').style.display = 'none';";
  html += "  document.getElementById('popup').style.display = 'none';";
  html += "}";
  html += "function confirmAction() {";
  html += "  var action = document.getElementById('popup').getAttribute('data-action');";
  html += "  window.location.href = '/' + action;";
  html += "}";
  html += "</script>";

  html += "</body></html>";
  server.send(200, "text/html", html);
}

void writeLedStateToFile(bool ledStatus, const char* fileName) {
  File file = SPIFFS.open(fileName, "w");
  if (file) {
    file.println(ledStatus);
    file.close();
  }
}

void handleLigar1() {
  digitalWrite(ledPin1, HIGH);
  ledStatus1 = true;
  writeLedStateToFile(ledStatus1, "/led_state1.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleDesligar1() {
  digitalWrite(ledPin1, LOW);
  ledStatus1 = false;
  writeLedStateToFile(ledStatus1, "/led_state1.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleLigar2() {
  digitalWrite(ledPin2, HIGH);
  ledStatus2 = true;
  writeLedStateToFile(ledStatus2, "/led_state2.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleDesligar2() {
  digitalWrite(ledPin2, LOW);
  ledStatus2 = false;
  writeLedStateToFile(ledStatus2, "/led_state2.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleLigar3() {
  digitalWrite(ledPin3, HIGH);
  ledStatus3 = true;
  writeLedStateToFile(ledStatus3, "/led_state3.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleDesligar3() {
  digitalWrite(ledPin3, LOW);
  ledStatus3 = false;
  writeLedStateToFile(ledStatus3, "/led_state3.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleLigar4() {
  digitalWrite(ledPin4, HIGH);
  ledStatus4 = true;
  writeLedStateToFile(ledStatus4, "/led_state4.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleDesligar4() {
  digitalWrite(ledPin4, LOW);
  ledStatus4 = false;
  writeLedStateToFile(ledStatus4, "/led_state4.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}
