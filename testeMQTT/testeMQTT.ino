#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// Configuração da rede WiFi
const char* ssid = "FallsBebidas";
const char* password = "87028302";

// Configuração do sensor de luminosidade LDR
const int ldrPin = A0;

// Configuração dos LEDs
const int redLedPin = 25;
const int greenLedPin = 26;
const int blueLedPin = 27;

// Configuração do cliente MQTT
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttClientID = "esp8266_client";
const char* temperatureTopic = "temperatura";
const char* luminosityTopic = "luminosidade";

// Variável para armazenar a temperatura recebida
float receivedTemperature = 0.0;

// Configuração do cliente WiFi e MQTT
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Função de callback para receber mensagens MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  // Converte o payload em uma string
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  String messageString(message);

  // Verifica se a mensagem é do tópico "temperatura"
  if (strcmp(topic, temperatureTopic) == 0) {
    // Obtém a temperatura recebida
    receivedTemperature = messageString.toFloat();
    Serial.print("Temperatura lida no tópico do Mqtt: ");
    Serial.println(receivedTemperature);
  }
}

// Função para conectar-se ao WiFi
void connectWiFi() {
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Conectado ao WiFi IP");
  Serial.print("Conectado ao WiFi");
  Serial.println(WiFi.localIP());
}

// Função para conectar-se ao broker MQTT
void connectMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  while (!mqttClient.connected()) {
    Serial.println("Conectando ao broker MQTT...");
    if (mqttClient.connect(mqttClientID)) {
      Serial.println("Conectado ao broker MQTT");

      // Assina o tópico "temperatura" para receber mensagens
      mqttClient.subscribe(temperatureTopic);
    } else {
      Serial.print("Falha na conexão MQTT, erro: ");
      Serial.print(mqttClient.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

// Função para publicar a leitura do sensor de luminosidade
void publishLuminosity() {
  // Lê a luminosidade do sensor LDR
  int luminosity = analogRead(ldrPin);
  Serial.print("Luminosidade lida e que será enviada ao mqtt: ");
  Serial.println(luminosity);
  // Publica a luminosidade no tópico "luminosidade"
  mqttClient.publish(luminosityTopic, String(luminosity).c_str());
  delay(5000);
}

// Função para controlar os LEDs com base na temperatura recebida
void controlLEDs() {
  if (receivedTemperature > 28) {
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);
    digitalWrite(blueLedPin, LOW);
    Serial.println("Dentro do IF de temperatura liga led vermelho");
  } else if (receivedTemperature > 20 && receivedTemperature <= 28) {
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(blueLedPin, LOW);
    Serial.println("Dentro do IF de temperatura liga led verde");
  } else {
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, LOW);
    digitalWrite(blueLedPin, HIGH);
    Serial.println("Dentro do IF de temperatura liga led azul");
  }
}

void setup() {
  // Inicializa a comunicação serial
  Serial.begin(115200);

  // Inicializa a conexão WiFi
  connectWiFi();

  // Inicializa os pinos dos LEDs como saída
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);

  // Inicializa a comunicação I2C
  //Wire.begin(D2, D1); // SDA - D2, SCL - D1

  // Conecta-se ao broker MQTT
  connectMQTT();
}

void loop() {
  // Verifica se a conexão WiFi está ativa
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conexão WiFi perdida. Reconectando...");
    connectWiFi();
  }

  // Verifica se a conexão MQTT está ativa
  if (!mqttClient.connected()) {
    Serial.println("Conexão MQTT perdida. Reconectando...");
    connectMQTT();
  }

  // Requisita a leitura da luminosidade e publica no broker MQTT
  publishLuminosity();

  // Verifica se há mensagens MQTT para processar
  mqttClient.loop();

  // Controla os LEDs com base na temperatura recebida
  controlLEDs();

  delay(5000);
}
