#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

const char* ssid = "FallsBebidas";
const char* password = "87028302";
const char* mqtt_server = "broker.emqx.io";
const char* mqtt_client_id = "esp32-client";

const char* mqtt_topic1 = "silvanojose/tomada1";
const char* mqtt_topic2 = "silvanojose/tomada2";
const char* mqtt_topic3 = "silvanojose/tomada3";
const char* mqtt_topic4 = "silvanojose/tomada4";
const char* mqtt_temp_box = "silvanojose/temperaturaBoxTomadas";

const int mqtt_port = 1883; // Porta padrão para MQTT

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const int sensorPin = 34; // Pino do sensor LM35 conectado ao ESP32
float temperature = 9999.0; // Variável global para armazenar a temperatura

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

unsigned long previousMillis = 0;
const long interval = 50000;

void setup_wifi() {
  delay(10);
  // Conexão Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi conectado.");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Mensagem recebida no tópico: ");
  //Serial.print(topic);
  //Serial.print(". Mensagem: ");

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  //Serial.println(topic);
  // Verificar qual tópico recebeu a mensagem
  if (String(topic) == "silvanojose/tomada1") {
    // Processar a mensagem para a Tomada 1
    int value = message.toInt();
    digitalWrite(ledPin1, value);
    ledStatus1 = value;
    writeLedStateToFile(ledStatus1, "/led_state1.txt");
    Serial.print("Callback recebeu notificação do tópico tomada1 :");
    Serial.println(value);
  }
  else if (String(topic) == "silvanojose/tomada2") {
    int value = message.toInt();
    digitalWrite(ledPin2, value);
    ledStatus2 = value;
    writeLedStateToFile(ledStatus2, "/led_state2.txt");
    Serial.print("Callback recebeu notificação do tópico tomada2 :");
    Serial.println(value);
  }
  // Adicionar código semelhante para outras tomadas...
  else if (String(topic) == "silvanojose/tomada3") {
    int value = message.toInt();
    digitalWrite(ledPin3, value);
    ledStatus3 = value;
    writeLedStateToFile(ledStatus3, "/led_state3.txt");
    Serial.print("Callback recebeu notificação do tópico tomada3 :");
    Serial.println(value);
  }
  else if (String(topic) == "silvanojose/tomada4") {
    int value = message.toInt();
    digitalWrite(ledPin4, value);
    ledStatus4 = value;
    writeLedStateToFile(ledStatus4, "/led_state4.txt");
    Serial.print("Callback recebeu notificação do tópico tomada4 :");
    Serial.println(value);
  }
  

}

void reconnect() {
  while (!client.connected()) {
    //Serial.print("Conectando ao broker MQTT...");
    if (client.connect("ESP32Client")) {
      //Serial.println("Conectado");
      //client.subscribe("internetecoisas/example/toggle"); // Subscreva ao tópico desejado aqui
      //client.subscribe("temperatura/teste-SJB");

      client.subscribe(mqtt_topic1, 2);
      client.subscribe(mqtt_topic2, 2);
      client.subscribe(mqtt_topic3, 2);
      client.subscribe(mqtt_topic4, 2);

    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void readTemperature() {
  int sensorValue = analogRead(sensorPin);
  temperature = (sensorValue / 4095.0) * 5000 / 10;
  Serial.print("Leu a temperatura....:");
  Serial.println(temperature);
  char tempStr[10]; // Cria um buffer para armazenar a temperatura como string
  dtostrf(temperature, 4, 2, tempStr); // Converte o valor float para uma string
//  client.publish(mqtt_temp_box, tempStr); // Publica a temperatura como uma string
  if (client.publish(mqtt_temp_box, tempStr, 2)) {
    Serial.println("Mensagem publicada com sucesso!");
  } else {
    Serial.println("Falha ao publicar a mensagem. Verifique sua conexão MQTT.");
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

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

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  server.on("/", handleRoot);
  
  server.on("/ligar1", handleLigar1);
  server.on("/desligar1", handleDesligar1);
  server.on("/ligar2", handleLigar2);
  server.on("/desligar2", handleDesligar2);
  server.on("/ligar3", handleLigar3);
  server.on("/desligar3", handleDesligar3);
  server.on("/ligar4", handleLigar4);
  server.on("/desligar4", handleDesligar4);

  /////////////////////////////////////////
  server.on("/cadastrarHorarios", HTTP_POST, [](){
    // Obter os parâmetros do formulário
    int dayOfWeek = server.arg("dayOfWeek").toInt();
    int hourOn = server.arg("hourOn").toInt();
    int minuteOn = server.arg("minuteOn").toInt();
    int hourOff = server.arg("hourOff").toInt();
    int minuteOff = server.arg("minuteOff").toInt();

    // Chamar a função para salvar o horário
    saveSchedule(dayOfWeek, hourOn, minuteOn, hourOff, minuteOff);

    // Redirecionar de volta para a página principal
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "Redirecting to /");
  });

  ////////////////////////////////////////

  server.begin();

}

void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();

  // Verifica se é hora de fazer a próxima leitura
  if (currentMillis - previousMillis >= interval) {
    // Salva o último tempo de leitura
    previousMillis = currentMillis;

    // Faça a leitura da temperatura
    readTemperature();
    checkSchedule();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  
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
  
  html += "<h1>Controle de Tomadas V3.2</h1>";
  
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

  // Adicionando o texto da temperatura
  html += "<p class='temperature'>Temperatura: ";
  html += temperature; // Inclua a variável temperature aqui
  html += " °C</p>";
 
html += "<h1>Cadastro de horários de acionamentos</h1>";
html += "<form action='/cadastrarHorarios' method='post'>";
html += "<label for='dayOfWeek'>Dia da semana:</label>";
html += "<select id='dayOfWeek' name='dayOfWeek'>";
html += "<option value='0'>Domingo</option>";
html += "<option value='1'>Segunda-feira</option>";
html += "<option value='2'>Terça-feira</option>";
html += "<option value='3'>Quarta-feira</option>";
html += "<option value='4'>Quinta-feira</option>";
html += "<option value='5'>Sexta-feira</option>";
html += "<option value='6'>Sábado</option>";
html += "</select><br><br>";
html += "<label for='hourOn'>Hora de ligar:</label>";
html += "<input type='number' id='hourOn' name='hourOn' min='0' max='23' required>";
html += "<label for='minuteOn'>Minuto de ligar:</label>";
html += "<input type='number' id='minuteOn' name='minuteOn' min='0' max='59' required><br><br>";
html += "<label for='hourOff'>Hora de desligar:</label>";
html += "<input type='number' id='hourOff' name='hourOff' min='0' max='23' required>";
html += "<label for='minuteOff'>Minuto de desligar:</label>";
html += "<input type='number' id='minuteOff' name='minuteOff' min='0' max='59' required><br><br>";
html += "<button class='btn' type='submit'>Cadastrar Horário</button>";



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
  client.publish(mqtt_topic1, "1", 2);
  digitalWrite(ledPin1, HIGH);
  ledStatus1 = true;
  writeLedStateToFile(ledStatus1, "/led_state1.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleDesligar1() {
  client.publish(mqtt_topic1, "0", 2);
  digitalWrite(ledPin1, LOW);
  ledStatus1 = false;
  writeLedStateToFile(ledStatus1, "/led_state1.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleLigar2() {
  client.publish(mqtt_topic2, "1", 2);
  digitalWrite(ledPin2, HIGH);
  ledStatus2 = true;
  writeLedStateToFile(ledStatus2, "/led_state2.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleDesligar2() {
  client.publish(mqtt_topic2, "0", 2);
  digitalWrite(ledPin2, LOW);
  ledStatus2 = false;
  writeLedStateToFile(ledStatus2, "/led_state2.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleLigar3() {
  client.publish(mqtt_topic3, "1", 2);
  digitalWrite(ledPin3, HIGH);
  ledStatus3 = true;
  writeLedStateToFile(ledStatus3, "/led_state3.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleDesligar3() {
  client.publish(mqtt_topic3, "0", 2);
  digitalWrite(ledPin3, LOW);
  ledStatus3 = false;
  writeLedStateToFile(ledStatus3, "/led_state3.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleLigar4() {
  client.publish(mqtt_topic4, "1", 2);
  digitalWrite(ledPin4, HIGH);
  ledStatus4 = true;
  writeLedStateToFile(ledStatus4, "/led_state4.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
}

void handleDesligar4() {
  client.publish(mqtt_topic4, "0", 2);
  digitalWrite(ledPin4, LOW);
  ledStatus4 = false;
  writeLedStateToFile(ledStatus4, "/led_state4.txt");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Redirecting to /");
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
      //Serial.println("Entrou no if 'file'...");
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
  Serial.print(" : ");
  Serial.print(currentMinute);
  Serial.print(" : ");
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
