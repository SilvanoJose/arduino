#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h> // Biblioteca necessária para manipulação de tempo
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "SJB";
const char* password = "56020399";
const char* mqtt_server = "broker.emqx.io";
const char* topic_temp_publish = "silvanojose/sensor/temperatura";
const char* topic_temp_subscribe = "temperaturaBoxTomadas";

WiFiClient espClient;
PubSubClient client(espClient);

const int lm35Pin = 34;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se à rede ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço de IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando se reconectar ao MQTT...");
    
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado");
      client.subscribe(topic_temp_subscribe);
    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  timeClient.begin();
  
  // Aguarda até que a hora seja obtida do servidor NTP
  while (!timeClient.update()) {
    timeClient.forceUpdate();
    delay(100);
  }
  
  Serial.println("Hora obtida do servidor NTP");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Obtenha a hora atual do servidor NTP
  timeClient.update();
  time_t now = timeClient.getEpochTime();
  
  Serial.print(analogRead("Valor lido no analogRead do sensor :");
  Serial.println(analogRead(lm35Pin));
  float tempC = analogRead(lm35Pin) * 0.48828125; // Converte valor analógico em temperatura (0.48828125 = 5000mV / 1024)

  // Construir a mensagem JSON
  DynamicJsonDocument jsonBuffer(256);
  jsonBuffer["timestamp"] = now; // Usar o timestamp do NTP
  jsonBuffer["temperatura"] = tempC;
  
  char buffer[256];
  serializeJson(jsonBuffer, buffer);
  
  Serial.print("Publicando no tópico: ");
  Serial.print(topic_temp_publish);
  Serial.print(" Mensagem: ");
  Serial.println(buffer);
  
  client.publish(topic_temp_publish, buffer);
  
  delay(15000); // Publica a cada 5 segundos
}
