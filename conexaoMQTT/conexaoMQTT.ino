#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "FallsBebidas";
const char* password = "87028302";
const char* mqtt_server = "192.168.0.104";

WiFiClient espClient;
PubSubClient client(espClient);

const char* topicPino1 = "casa/pino1";
const char* topicPino2 = "casa/pino2";
const char* topicPino3 = "casa/pino3";
const char* topicPino4 = "casa/pino4";

int pino1 = 2;  // Pino GPIO2 no ESP32
int pino2 = 4;  // Pino GPIO4 no ESP32
int pino3 = 5;  // Pino GPIO5 no ESP32
int pino4 = 18; // Pino GPIO18 no ESP32

void setup_wifi() {
  delay(10);
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  Serial.println("Conectado ao WiFi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  // Converter o payload para uma string
  String payloadStr = String((char*)payload);

  Serial.print("Payload: ");
  Serial.println(payloadStr);

  // Atualizar o estado dos pinos com base na mensagem MQTT recebida
  if (strncmp(topic, topicPino1, strlen(topicPino1)) == 0) {
    digitalWrite(pino1, payloadStr.toInt());
  } else if (strncmp(topic, topicPino2, strlen(topicPino2)) == 0) {
    digitalWrite(pino2, payloadStr.toInt());
  } else if (strncmp(topic, topicPino3, strlen(topicPino3)) == 0) {
    digitalWrite(pino3, payloadStr.toInt());
  } else if (strncmp(topic, topicPino4, strlen(topicPino4)) == 0) {
    digitalWrite(pino4, payloadStr.toInt());
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando reconectar ao MQTT...");

    if (client.connect("ESP32Client")) {
      Serial.println("Conectado ao MQTT");

      // Inscrever-se nos tópicos de controle
      client.subscribe(topicPino1);
      client.subscribe(topicPino2);
      client.subscribe(topicPino3);
      client.subscribe(topicPino4);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(pino1, OUTPUT);
  pinMode(pino2, OUTPUT);
  pinMode(pino3, OUTPUT);
  pinMode(pino4, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // Ler o estado dos pinos e publicar no MQTT
  client.publish(topicPino1, String(digitalRead(pino1)).c_str());
  client.publish(topicPino2, String(digitalRead(pino2)).c_str());
  client.publish(topicPino3, String(digitalRead(pino3)).c_str());
  client.publish(topicPino4, String(digitalRead(pino4)).c_str());

  delay(1000); // Aguardar 1 segundo
}
