#include <SPIFFS.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

//Varíaveis sensor temperatura
char msgTemperatura[10];
float temperatura;

// WiFi configuration
const char* ssid = "FallsBebidas";
const char* password = "87028302";

// MQTT configuration
const char* mqtt_server = "broker.emqx.io";
//const char* mqtt_server = "192.168.0.104";
const int sensorPin = 34; // Pino do sensor LM35 conectado ao ESP32
const int mqtt_port = 1883;
unsigned long lastMsg = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const int ledPin1 = 27;
const int ledPin2 = 26;
const int ledPin3 = 32;
const int ledPin4 = 33;
bool ledStatus1 = false;
bool ledStatus2 = false;
bool ledStatus3 = false;
bool ledStatus4 = false;

const char* mqtt_topic1 = "silvanojose/tomada1";
const char* mqtt_topic2 = "silvanojose/tomada2";
const char* mqtt_topic3 = "silvanojose/tomada3";
const char* mqtt_topic4 = "silvanojose/tomada4";
const char* mqtt_temp_box = "silvanojose/temperaturaBoxTomadas";
const char* mqtt_schedule = "silvanojose/schedule";
const char* mqtt_status = "silvanojose/statusTomadas";

WiFiClient espClient;
PubSubClient client(espClient);

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

//Função callback chamada quando uma mensagem for recebida para subscrições:
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  Serial.print("Mensagem chegando [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();

  // Verificar qual tópico recebeu a mensagem
  if (String(topic) == "silvanojose/temperaturaBoxTomadas") {
    //Serial.print("Callback recebeu notificação do tópico silvanojose/temperaturaBoxTomadas :");
    //Serial.println(message);
  }
  else if (String(topic) == "silvanojose/tomada1") {
    // Processar a mensagem para a Tomada 1
    int value = message.toInt();
    digitalWrite(ledPin1, value);
    ledStatus1 = value;
    writeLedStateToFile(ledStatus1, "/led_state1.txt");
    //Serial.print("Callback recebeu notificação do tópico tomada1 :");
    //Serial.println(value);
  }
  else if (String(topic) == "silvanojose/tomada2") {
    int value = message.toInt();
    digitalWrite(ledPin2, value);
    ledStatus2 = value;
    writeLedStateToFile(ledStatus2, "/led_state2.txt");
    //Serial.print("Callback recebeu notificação do tópico tomada2 :");
    //Serial.println(value);
  }
  // Adicionar código semelhante para outras tomadas...
  else if (String(topic) == "silvanojose/tomada3") {
    int value = message.toInt();
    digitalWrite(ledPin3, value);
    ledStatus3 = value;
    writeLedStateToFile(ledStatus3, "/led_state3.txt");
    //Serial.print("Callback recebeu notificação do tópico tomada3 :");
    //Serial.println(value);
  }
  else if (String(topic) == "silvanojose/tomada4") {
    int value = message.toInt();
    digitalWrite(ledPin4, value);
    ledStatus4 = value;
    writeLedStateToFile(ledStatus4, "/led_state4.txt");
    //Serial.print("Callback recebeu notificação do tópico tomada4 :");
    //Serial.println(value);
  }
  else if (String(topic) == "silvanojose/statusTomadas") {
    //Serial.print("Mensagem do tópico silvanojose/statusTomadas: ");
    //Serial.println(message);
  }
  else if (String(topic) == "silvanojose/schedule") {
    // Extrair dados do payload (supondo que a mensagem tenha o formato "dia,horaOn,minutoOn,horaOff,minutoOff")
    int dayOfWeek = message.substring(0, message.indexOf(',')).toInt();
    message.remove(0, message.indexOf(',') + 1);
    int hourOn = message.substring(0, message.indexOf(',')).toInt();
    message.remove(0, message.indexOf(',') + 1);
    int minuteOn = message.substring(0, message.indexOf(',')).toInt();
    message.remove(0, message.indexOf(',') + 1);
    int hourOff = message.substring(0, message.indexOf(',')).toInt();
    message.remove(0, message.indexOf(',') + 1);
    int minuteOff = message.toInt();
    
    // Chamar a função para salvar o cronograma
    saveSchedule(dayOfWeek, hourOn, minuteOn, hourOff, minuteOff);
  }
}

void setup(){
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

    // Inicializa o cliente NTP
  timeClient.begin();
  // Aguarda a conexão ser estabelecida
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  Serial.println("Conectado ao servidor NTP!");

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

  readSchedule();

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

}

void loop() {
  if (!client.connected()) {
    Serial.println("MQTT não conectado, vai chamar reconect..");
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 55000) {
    lastMsg = now;
    readTemperature(); 
    checkSchedule();
    Serial.println("---Ciclos dentro do loop");
  }
}

void reconnect() {
  //Mensagem lastWill
  byte willQoS = 0;
  const char* willTopic = mqtt_status;
  const char* willMessage = "OFF_LINE";
  boolean willRetain = true;
  // Loop até reconectar
  while (!client.connected()) {
    Serial.print("Reconectando MQTT...");
    // Cria identificação randômica do cliente
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), willTopic, willQoS, willRetain, willMessage)) {
      Serial.println("Conectado..");
      // Uma vez conectado publica anúncio:
      char* message = "ON_LINE";
      int length = strlen(message);
      boolean retained = true; 
      client.publish(mqtt_status, (byte*)message, length, retained);
      // Subscreve tópico
      client.subscribe(mqtt_topic1);
      client.subscribe(mqtt_topic2);
      client.subscribe(mqtt_topic3);
      client.subscribe(mqtt_topic4);
      client.subscribe(mqtt_temp_box);
      client.subscribe(mqtt_status);
      client.subscribe(mqtt_schedule);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishTemperature() {
  // Simulated temperature reading
  float temperature = analogRead(A0) * 0.48875855327;
  char tempStr[10];
  dtostrf(temperature, 4, 2, tempStr);
  if (client.publish(mqtt_temp_box, tempStr, true)) {
    Serial.println("Mensagem temperatura publicada com sucesso! " + String(temperature));
  } else {
    Serial.println("Falha ao publicar mensagem temperatura!");
  }
}

void publishStatus() {
  // Read the status of the digital pin and publish it
  int status = digitalRead(ledPin1);
  if (client.publish(mqtt_topic1, String(status).c_str(), true)) {
    Serial.println("Mensagem Status publicada com sucesso! " + String(status));
  } else {
    Serial.println("Falha ao publicar mensagem de status!");
  }
}

void readTemperature() {
  float temperature = 9999.0;
  int sensorValue = analogRead(sensorPin);
  temperature = (sensorValue / 4095.0) * 5000 / 10;
  Serial.print("Leu a temperatura....:");
  Serial.println(temperature);
  char tempStr[10]; // Cria um buffer para armazenar a temperatura como string
  dtostrf(temperature, 4, 2, tempStr); // Converte o valor float para uma string
//  client.publish(mqtt_temp_box, tempStr); // Publica a temperatura como uma string
  if (client.publish(mqtt_temp_box, tempStr)) {
    Serial.println("Mensagem publicada com sucesso!");
  } else {
    Serial.println("Falha ao publicar a mensagem. Verifique sua conexão MQTT.");
  }
}

void writeLedStateToFile(bool ledStatus, const char* fileName) {
  File file = SPIFFS.open(fileName, "w");
  if (file) {
    file.println(ledStatus);
    file.close();
  }
}

void handleLigar1() {
  client.publish(mqtt_topic1, "1");
  digitalWrite(ledPin1, HIGH);
  ledStatus1 = true;
  writeLedStateToFile(ledStatus1, "/led_state1.txt");
}

void handleDesligar1() {
  client.publish(mqtt_topic1, "0");
  digitalWrite(ledPin1, LOW);
  ledStatus1 = false;
  writeLedStateToFile(ledStatus1, "/led_state1.txt");
}

void handleLigar2() {
  client.publish(mqtt_topic2, "1");
  digitalWrite(ledPin2, HIGH);
  ledStatus2 = true;
  writeLedStateToFile(ledStatus2, "/led_state2.txt");
}

void handleDesligar2() {
  client.publish(mqtt_topic2, "0");
  digitalWrite(ledPin2, LOW);
  ledStatus2 = false;
  writeLedStateToFile(ledStatus2, "/led_state2.txt");
}

void handleLigar3() {
  client.publish(mqtt_topic3, "1");
  digitalWrite(ledPin3, HIGH);
  ledStatus3 = true;
  writeLedStateToFile(ledStatus3, "/led_state3.txt");
}

void handleDesligar3() {
  client.publish(mqtt_topic3, "0");
  digitalWrite(ledPin3, LOW);
  ledStatus3 = false;
  writeLedStateToFile(ledStatus3, "/led_state3.txt");
}

void handleLigar4() {
  client.publish(mqtt_topic4, "1");
  digitalWrite(ledPin4, HIGH);
  ledStatus4 = true;
  writeLedStateToFile(ledStatus4, "/led_state4.txt");
}

void handleDesligar4() {
  client.publish(mqtt_topic4, "0");
  digitalWrite(ledPin4, LOW);
  ledStatus4 = false;
  writeLedStateToFile(ledStatus4, "/led_state4.txt");
}

void saveSchedule(int dayOfWeek, int hourOn, int minuteOn, int hourOff, int minuteOff) {
  // Formatar o nome do arquivo com base no dia da semana
  String fileName = "/schedule_" + String(dayOfWeek) + ".txt";

  // Abre o arquivo para escrita em modo de truncamento
  File file = SPIFFS.open(fileName, "w");
  if (!file) {
    Serial.println("Falha ao abrir o arquivo para escrita.");
    return;
  }

  // Escrever os dados no arquivo
  file.print(hourOn < 10 ? "0" + String(hourOn) : String(hourOn)); // Adiciona um zero à esquerda se a hora for menor que 10
  file.print(":");
  file.print(minuteOn < 10 ? "0" + String(minuteOn) : String(minuteOn)); // Adiciona um zero à esquerda se os minutos forem menores que 10
  file.print("-");
  file.print(hourOff < 10 ? "0" + String(hourOff) : String(hourOff)); // Adiciona um zero à esquerda se a hora for menor que 10
  file.print(":");
  file.println(minuteOff < 10 ? "0" + String(minuteOff) : String(minuteOff)); // Adiciona um zero à esquerda se os minutos forem menores que 10

  // Fechar o arquivo
  file.close();

  Serial.println("Horário salvo com sucesso.");
}

void readSchedule() {
  Serial.println("Lendo horários salvos...");
  for (int dayOfWeek = 0; dayOfWeek <= 6; dayOfWeek++) {
    String fileName = "/schedule_" + String(dayOfWeek) + ".txt";
    File file = SPIFFS.open(fileName, "r");
    if (file) {
      while (file.available()) {
        String line = file.readStringUntil('\n');
        int hourOn = line.substring(0, 2).toInt();
        int minuteOn = line.substring(3, 5).toInt();
        int hourOff = line.substring(6, 8).toInt();
        int minuteOff = line.substring(9, 11).toInt();
        // Aqui você pode fazer o que quiser com os dados lidos
        Serial.print("Dia da semana: ");
        Serial.println(dayOfWeek);
        Serial.print("Hora de ligar: ");
        Serial.print(hourOn);
        Serial.print(":");
        Serial.print(minuteOn);
        Serial.print(" - Hora de desligar: ");
        Serial.print(hourOff);
        Serial.print(":");
        Serial.println(minuteOff);
      }
      file.close();
    } else {
      Serial.println("Erro ao abrir o arquivo " + fileName);
    }
  }
}

void checkSchedule() {

  // Atualiza o cliente NTP para obter a hora atual
  timeClient.update();
  
  // Obtém a hora atual
  unsigned long epochTime = timeClient.getEpochTime();
  // Converte a hora UTC para o fuso horário local (UTC+3)
  unsigned long localEpochTime = epochTime - (3 * 3600); // Adiciona 3 horas em segundos
  // Converte o tempo local para o tipo time_t
  time_t currentTime = (time_t)localEpochTime; // Convertendo para o tipo time_t
  
  struct tm *timeInfo;
  timeInfo = localtime(&currentTime);
  
  // Extrai o dia da semana (0 - domingo, 1 - segunda-feira, ..., 6 - sábado)
  int currentDayOfWeek = timeInfo->tm_wday;  
  //int currentDayOfWeek = weekday();
  
  // Formatar o nome do arquivo com base no dia da semana atual
  String fileName = "/schedule_" + String(currentDayOfWeek) + ".txt";

  // Abrir o arquivo para leitura
  File file = SPIFFS.open(fileName, "r");
  if (!file) {
    Serial.println("Falha ao abrir o arquivo para leitura.");
    return;
  }

  // Ler o horário do arquivo
  String schedule = file.readStringUntil('\n');
  file.close();

  int currentHour = timeInfo->tm_hour;
  int currentMinute = timeInfo->tm_min;
  int currentSecond = timeInfo->tm_sec;

  // Exibe a hora atual e o dia da semana no monitor serial
  Serial.print("Hora: ");
  Serial.print(currentHour);
  Serial.print(":");
  Serial.print(currentMinute);
  Serial.print(":");
  Serial.print(currentSecond);
  Serial.print(", Dia da semana: ");
  Serial.println(currentDayOfWeek);
  
  // Converter a string de horário lida para horas e minutos
  int hourOn = schedule.substring(0, 2).toInt();
  int minuteOn = schedule.substring(3, 5).toInt();
  int hourOff = schedule.substring(6, 8).toInt();
  int minuteOff = schedule.substring(9, 11).toInt();

  // Verificar se está na hora de ligar ou desligar
  if (currentHour == hourOn && currentMinute == minuteOn) {
    // Hora de ligar
    Serial.println("Entrou na hora de ligar..");
    handleLigar1();
    handleLigar2();
    handleLigar3();
    handleLigar4();
  } else if (currentHour == hourOff && currentMinute == minuteOff) {
    // Hora de desligar
    Serial.println("Entrou na hora de Desligar..");
    handleDesligar1();
    handleDesligar2();
    handleDesligar3();
    handleDesligar4();
  }
}

