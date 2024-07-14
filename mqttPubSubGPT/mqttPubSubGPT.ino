#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "FallsBebidas";
const char* password = "87028302";
const char* mqtt_server = "mqtt.internetecoisas.com.br"; // Substitua pelo endereço do seu servidor MQTT

WiFiClient espClient;
PubSubClient client(espClient);

const int ledPin1 = 27;
bool ledStatus1 = false;

void setup() {
  Serial.begin(115200);

  pinMode(ledPin1, OUTPUT);
  digitalWrite(ledPin1, LOW);

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando-se à rede Wi-Fi...");
  }

  // Configurar MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Verificar qual tópico recebeu a mensagem
  if (String(topic) == "internetecoisas/example/toggle") {
    // Processar a mensagem para a Tomada 1
    int value = message.toInt();
    digitalWrite(ledPin1, value);
    ledStatus1 = value;
    Serial.println("internetecoisas/example/toggle recebeu :");
    ledStatus1 = value;
    Serial.println(value);
  }
  if (String(topic) == "temperatura/teste-SJB") {
    // Processar a mensagem para a Tomada 1
    int value = message.toInt();
    digitalWrite(ledPin1, value);
    ledStatus1 = value;
    Serial.println("temperatura/teste-SJB recebeu :");
    ledStatus1 = value;
    Serial.println(value);
  }
  // Adicionar código semelhante para outras tomadas...
}

void reconnect() {
  // Loop até que estejamos reconectados
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    // Tentar se reconectar
    if (client.connect("ESP32Client")) {
      Serial.println("conectado");
      // Inscrever nos tópicos
      client.subscribe("internetecoisas/example/toggle");
      // Inscrever em outros tópicos...
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentar novamente em 5 segundos");
      // Aguardar 5 segundos antes de tentar novamente
      delay(5000);
    }
  }
}

