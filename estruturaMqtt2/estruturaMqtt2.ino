#include <WiFi.h>
#include <PubSubClient.h>

// WiFi configuration
const char* ssid = "FallsBebidas";
const char* password = "87028302";

// MQTT configuration
//const char* mqtt_server = "broker.emqx.io";
const char* mqtt_server = "192.168.0.105";
const int mqtt_port = 1883;
const char* mqtt_user = "USUARIO_MQTT";
const char* mqtt_password = "SENHA_MQTT";
const char* temperature_topic = "casa/temperatura";
const char* status_topic = "casa/status";

// Pin for digital control
const int control_pin = 23; // Exemplo de pino para ESP32, altere conforme necessário

WiFiClient espClient;
PubSubClient client(espClient);

// Function prototypes
void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void publishTemperature();
void publishStatus();

void setup() {
  pinMode(control_pin, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  publishTemperature();
  publishStatus();
  client.loop();
  Serial.println("-----------------------------------");
  delay(10000); // Atraso de 5 segundos
}

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
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(status_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Verificar qual tópico recebeu a mensagem
  if (String(topic) == "casa/temperatura") {
    Serial.print("Callback recebeu notificação do tópico casa/temperatura :");
    Serial.println(message);
  }
  else if (String(topic) == "casa/status") {
    Serial.print("Callback recebeu notificação do tópico casa/status :");
    Serial.println(message);
  }
}

void publishTemperature() {
  // Simulated temperature reading
  float temperature = analogRead(A0) * 0.48875855327;
  char tempStr[10];
  dtostrf(temperature, 4, 2, tempStr);
  if (client.publish(temperature_topic, tempStr, true)) {
    Serial.println("Mensagem temperatura publicada com sucesso! " + String(temperature));
  } else {
    Serial.println("Falha ao publicar mensagem temperatura!");
  }
}

void publishStatus() {
  // Read the status of the digital pin and publish it
  int status = digitalRead(control_pin);
  if (client.publish(status_topic, String(status).c_str(), true)) {
    Serial.println("Mensagem Status publicada com sucesso! " + String(status));
  } else {
    Serial.println("Falha ao publicar mensagem de status!");
  }
}

