#include <SPIFFS.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <vector>
#include "esp_task_wdt.h" // Inclui a biblioteca para o watchdog timer
#include <vector>
#include <map>

// WiFi configuration
const char* ssid = "FallsBebidas";
const char* password = "87028302";
//const char* ssid = "IFPR-Alunos";
//const char* password = "";

// MQTT configuration
const char* mqtt_server = "mqtt.eclipseprojects.io";
const int mqtt_port = 1883;
const char* mqtt_user = "USUARIO_MQTT";
const char* mqtt_password = "SENHA_MQTT";
const char* mqtt_topic1 = "silvanojose/tomada1";
const char* mqtt_topic2 = "silvanojose/tomada2";
const char* mqtt_topic3 = "silvanojose/tomada3";
const char* mqtt_topic4 = "silvanojose/tomada4";
const char* mqtt_temp_box = "silvanojose/temperaturaBoxTomadas";
const char* mqtt_schedule = "silvanojose/schedule";
const char* mqtt_status = "silvanojose/statusTomadas";
const char* mqtt_topic_format = "silvanojose/format";
const char* mqtt_topic_print = "silvanojose/serialprints";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const int sensorPin = 34; // Pino do sensor LM35 conectado ao ESP32
float temperature = 9999.0; // Variável global para armazenar a temperatura

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

const int ledPins[4] = {26, 27, 32, 33};
bool ledStatus[4] = {false, false, false, false};

struct Schedule {
  int hourOn;
  int minuteOn;
  int hourOff;
  int minuteOff;
};

// Mapa de agendamentos: dia da semana -> tomada -> lista de agendamentos
std::map<int, std::map<int, std::vector<Schedule>>> schedules;

// Function prototypes
void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void readTemperature();
void checkSchedules();
void saveSchedules(int dayOfWeek, int pin, int scheduleIndex, int hourOn, int minuteOn, int hourOff, int minuteOff);
void readSchedules();
void handleLigar(int tomada);
void handleDesligar(int tomada);
void writeLedStateToFile(bool state, const char* fileName);
void restoreLedState();

void publishSerialPrint(String messageTopic) {
    if (client.connected()) {
        bool success = client.publish("silvanojose/serialprints", messageTopic.c_str());
        if (!success) {
            Serial.println("Falha ao publicar a mensagem: " + messageTopic);
        }
    } else {
        Serial.println("MQTT não está conectado, mensagem não publicada: " + messageTopic);
    }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Inicializa o cliente NTP
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  Serial.println("Conectado ao servidor NTP!");

  for (int i = 0; i < 4; i++) {
    Serial.print("Dentro do setup, carregamento dos pinos ");
    Serial.println(ledPins[i]);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  if (!SPIFFS.begin(true)) {
    Serial.println("Ocorreu um erro ao montar o SPIFFS");
    return;
  }

  readSchedules();
  restoreLedState();
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
    checkSchedules();
    //Serial.println("---Ciclos dentro do loop");
  }
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
  //Mensagem lastWill
  byte willQoS = 0;
  const char* willTopic = mqtt_status;
  const char* willMessage = "OFF_LINE";
  boolean willRetain = true;
  // Loop até reconectar
  while (!client.connected()) {
    Serial.println("Reconectando MQTT...");
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
      client.subscribe(mqtt_topic_format);
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
    Serial.print("Mensagem recebida [");
    Serial.print(topic);
    Serial.print("] ");

    String messageTopic = "Mensagem [" + String(topic) + "] ";
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        message += (char)payload[i];
    }
    Serial.println();

    // Verificar qual tópico recebeu a mensagem
    int tomada = -1;
    if (String(topic) == mqtt_topic1) tomada = 0;
    else if (String(topic) == mqtt_topic2) tomada = 1;
    else if (String(topic) == mqtt_topic3) tomada = 2;
    else if (String(topic) == mqtt_topic4) tomada = 3;

    if (tomada != -1) {
        Serial.println("Mensagem para ligar/desligar recebida.");
        int value = message.toInt();
        Serial.print("LedPin número: ");
        Serial.println(ledPins[tomada]);

        // Acionar o pino diretamente
        manipularPino(tomada, value);

        Serial.print("Callback processou o comando para tomada ");
        Serial.print(tomada + 1);
        Serial.print(" com valor ");
        Serial.println(value);
    } 
    else if (String(topic) == "silvanojose/schedule") {
        Serial.println("Mensagem de agendamento recebida.");
        
        // Extrair dados do payload
        int dayOfWeek = message.substring(0, message.indexOf(',')).toInt();
        message.remove(0, message.indexOf(',') + 1);
        int pin = message.substring(0, message.indexOf(',')).toInt();
        message.remove(0, message.indexOf(',') + 1);
        int scheduleIndex = message.substring(0, message.indexOf(',')).toInt();
        message.remove(0, message.indexOf(',') + 1);
        int hourOn = message.substring(0, message.indexOf(',')).toInt();
        message.remove(0, message.indexOf(',') + 1);
        int minuteOn = message.substring(0, message.indexOf(',')).toInt();
        message.remove(0, message.indexOf(',') + 1);
        int hourOff = message.substring(0, message.indexOf(',')).toInt();
        message.remove(0, message.indexOf(',') + 1);
        int minuteOff = message.substring(0).toInt();

        // Exibir os dados extraídos
        Serial.println("Salvando agendamento com os seguintes dados:");
        Serial.print("Dia da semana: ");
        Serial.print(dayOfWeek);
        Serial.print(", Tomada: ");
        Serial.print(pin);
        Serial.print(", Índice do agendamento: ");
        Serial.print(scheduleIndex);
        Serial.print(", Hora de ligar: ");
        Serial.print(hourOn);
        Serial.print(":");
        Serial.print(minuteOn);
        Serial.print(", Hora de desligar: ");
        Serial.print(hourOff);
        Serial.print(":");
        Serial.println(minuteOff);

        // Salvar e recarregar cronogramas
        saveSchedules(dayOfWeek, pin, scheduleIndex, hourOn, minuteOn, hourOff, minuteOff);
        readSchedules(); // Recarregar os agendamentos
    } 
    else if (String(topic) == mqtt_topic_format) {
        if (message == "format") {
            Serial.println("Mensagem de formatação recebida.");
            formatSPIFFS();
        }
    }

    // Publicar a mensagem recebida no tópico de prints
    messageTopic += message;
    publishSerialPrint(messageTopic);
}



void formatSPIFFS() {
  Serial.println("Formatando SPIFFS...");
// Reinicia o watchdog timer para evitar que ele expire durante a formatação
  esp_task_wdt_reset();  
  SPIFFS.format();
  // Após a formatação, reinicia o watchdog timer novamente
  esp_task_wdt_reset();
  Serial.println("SPIFFS formatado com sucesso.");
}

void readTemperature() {
  float temperature = 9999.0;
  int sensorValue = analogRead(sensorPin);
  temperature = (sensorValue / 4095.0) * 5000 / 10;
  //Serial.print("Leu a temperatura....:");
  //Serial.println(temperature);
  char tempStr[10]; // Cria um buffer para armazenar a temperatura como string
  dtostrf(temperature, 4, 2, tempStr); // Converte o valor float para uma string
//  client.publish(mqtt_temp_box, tempStr); // Publica a temperatura como uma string
  if (client.publish(mqtt_temp_box, tempStr)) {
    //Serial.println("Mensagem publicada com sucesso!");
  } else {
    Serial.println("Falha ao publicar a mensagem. Verifique sua conexão MQTT.");
  }
}

void checkSchedules() {
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    unsigned long localEpochTime = epochTime - (3 * 3600); // Ajuste para o fuso horário local
    time_t currentTime = (time_t)localEpochTime;
    struct tm *timeInfo = localtime(&currentTime);
    int currentDayOfWeek = timeInfo->tm_wday;
    int currentHour = timeInfo->tm_hour;
    int currentMinute = timeInfo->tm_min;
    int currentSecond = timeInfo->tm_sec;

    // Exibe a hora atual e o dia da semana no monitor serial
    Serial.println("Entrou no checkSchedules...");
    Serial.print("Hora: ");
    Serial.print((currentHour < 10 ? "0" : "") + String(currentHour));
    Serial.print(":");
    Serial.print((currentMinute < 10 ? "0" : "") + String(currentMinute));
    Serial.print(":");
    Serial.print((currentSecond < 10 ? "0" : "") + String(currentSecond));
    Serial.print(", Dia da semana: ");
    Serial.println(currentDayOfWeek);

    // Verifica se há agendamentos para o dia atual
    if (schedules.find(currentDayOfWeek) != schedules.end()) {
        for (int tomada = 0; tomada < 4; tomada++) { // Tomadas 0 a 3
            // Verifica se há agendamentos para a tomada atual
            if (schedules[currentDayOfWeek].find(tomada) != schedules[currentDayOfWeek].end()) {
                int scheduleIndex = 0; // Inicializa o índice do agendamento
                for (const auto &schedule : schedules[currentDayOfWeek][tomada]) {
                    if (scheduleIndex >= 3) break; // Limite de 3 agendamentos
                    Serial.print("Verificando tomada ");
                    Serial.print(tomada); // Exibe tomada no intervalo 0 a 3
                    Serial.print(", Agendamento ");
                    Serial.print(scheduleIndex); // Exibe índice de agendamento
                    Serial.print(": ");
                    Serial.print((schedule.hourOn < 10 ? "0" : "") + String(schedule.hourOn));
                    Serial.print(":");
                    Serial.print((schedule.minuteOn < 10 ? "0" : "") + String(schedule.minuteOn));
                    Serial.print(" - ");
                    Serial.print((schedule.hourOff < 10 ? "0" : "") + String(schedule.hourOff));
                    Serial.print(":");
                    Serial.println((schedule.minuteOff < 10 ? "0" : "") + String(schedule.minuteOff));

                    // Verifica se o horário atual coincide com o horário de ligar
                    if (currentHour == schedule.hourOn && currentMinute == schedule.minuteOn) {
                        if (!ledStatus[tomada]) { // Verifica se a tomada está desligada
                            Serial.print("Ligando tomada ");
                            Serial.println(tomada);
                            handleLigar(tomada);
                        }
                    }
                    // Verifica se o horário atual coincide com o horário de desligar
                    else if (currentHour == schedule.hourOff && currentMinute == schedule.minuteOff) {
                        if (ledStatus[tomada]) { // Verifica se a tomada está ligada
                            Serial.print("Desligando tomada ");
                            Serial.println(tomada);
                            handleDesligar(tomada);
                        }
                    }

                    scheduleIndex++; // Incrementa o índice do agendamento
                }
            } else {
                Serial.print("Nenhum agendamento para tomada ");
                Serial.println(tomada);
            }
        }
    } else {
        Serial.println("Nenhum agendamento para o dia atual.");
    }
}



void saveSchedules(int dayOfWeek, int pin, int scheduleIndex, int hourOn, int minuteOn, int hourOff, int minuteOff) {
    String fileName = "/schedule_" + String(dayOfWeek) + ".txt";
    String newLine = String(pin) + "," + String(scheduleIndex) + "," +
                     (hourOn < 10 ? "0" : "") + String(hourOn) + ":" + (minuteOn < 10 ? "0" : "") + String(minuteOn) + "," +
                     (hourOff < 10 ? "0" : "") + String(hourOff) + ":" + (minuteOff < 10 ? "0" : "") + String(minuteOff) + "\n";
    String fileContent = "";
    bool scheduleExists = false;

    Serial.println("Salvando agendamento para o dia da semana: " + String(dayOfWeek));
    Serial.println("Novo agendamento: " + newLine);

    // Ler horários existentes
    File file = SPIFFS.open(fileName, "r");
    if (file) {
        while (file.available()) {
            String line = file.readStringUntil('\n');
            if (!line.isEmpty()) {
                int comma1 = line.indexOf(',');
                int comma2 = line.indexOf(',', comma1 + 1);
                int existingPin = line.substring(0, comma1).toInt(); // Nenhum ajuste, valores já de 0 a 3
                int existingScheduleIndex = line.substring(comma1 + 1, comma2).toInt();

                Serial.println("Horário existente encontrado: " + line);
                Serial.println("Pin existente: " + String(existingPin) + ", ScheduleIndex existente: " + String(existingScheduleIndex));
                Serial.println("Comparando com Pin: " + String(pin) + ", ScheduleIndex: " + String(scheduleIndex));

                // Substituir se já existir um agendamento com o mesmo pin e scheduleIndex
                if (existingPin == pin && existingScheduleIndex == scheduleIndex) {
                    fileContent += newLine; // Substituir o agendamento existente
                    scheduleExists = true;
                    Serial.println("Agendamento existente substituído.");
                } else {
                    fileContent += line + "\n"; // Manter o agendamento existente
                }
            }
        }
        file.close();
    } else {
        Serial.println("Arquivo não encontrado, criando novo.");
    }

    // Adicionar novo agendamento se ele não existir
    if (!scheduleExists) {
        fileContent += newLine;
        Serial.println("Novo agendamento adicionado.");
    }

    // Escrever horários atualizados de volta no arquivo
    file = SPIFFS.open(fileName, "w");
    if (file) {
        file.print(fileContent);
        file.close();
        Serial.println("Horário salvo com sucesso.");
    } else {
        Serial.println("Erro ao abrir o arquivo " + fileName + " para escrita.");
    }
}


void readSchedules() {
    Serial.println("-------------------------------------Iniciando a leitura dos agendamentos salvos...readSchedule");

    for (int dayOfWeek = 0; dayOfWeek <= 6; dayOfWeek++) {
        String fileName = "/schedule_" + String(dayOfWeek) + ".txt";
        File file = SPIFFS.open(fileName, "r");

        if (file) {
            if (!file.available()) {
                Serial.println("Nenhum agendamento salvo para o dia " + String(dayOfWeek) + ".");
            } else {
                int scheduleCount = 0; // Limitar a leitura a 3 agendamentos por dia
                while (file.available() && scheduleCount < 3) {
                    String line = file.readStringUntil('\n');

                    // Verificar o formato da linha
                    int comma1 = line.indexOf(',');
                    int comma2 = line.indexOf(',', comma1 + 1);
                    int colon1 = line.indexOf(':', comma2 + 1);
                    int comma3 = line.indexOf(',', colon1 + 2);
                    int colon2 = line.indexOf(':', comma3 + 1);

                    if (comma1 != -1 && comma2 != -1 && colon1 != -1 && comma3 != -1 && colon2 != -1) {
                        int pin = line.substring(0, comma1).toInt();
                        int scheduleIndex = line.substring(comma1 + 1, comma2).toInt();
                        int hourOn = line.substring(comma2 + 1, colon1).toInt();
                        int minuteOn = line.substring(colon1 + 1, comma3).toInt();
                        int hourOff = line.substring(comma3 + 1, colon2).toInt();
                        int minuteOff = line.substring(colon2 + 1).toInt();

                        // Formatar os valores com zeros à esquerda
                        String formattedHourOn = (hourOn < 10 ? "0" : "") + String(hourOn);
                        String formattedMinuteOn = (minuteOn < 10 ? "0" : "") + String(minuteOn);
                        String formattedHourOff = (hourOff < 10 ? "0" : "") + String(hourOff);
                        String formattedMinuteOff = (minuteOff < 10 ? "0" : "") + String(minuteOff);

                        // Exibição dos dados lidos em uma única linha
                        Serial.print("Dia: ");
                        Serial.print(dayOfWeek);
                        Serial.print(", Tomada: ");
                        Serial.print(pin);
                        Serial.print(", Agendamento: ");
                        Serial.print(scheduleIndex);
                        Serial.print(", Liga às: ");
                        Serial.print(formattedHourOn);
                        Serial.print(":");
                        Serial.print(formattedMinuteOn);
                        Serial.print(", Desliga às: ");
                        Serial.print(formattedHourOff);
                        Serial.print(":");
                        Serial.println(formattedMinuteOff);

                        scheduleCount++; // Incrementar o número de agendamentos lidos
                    } else {
                        Serial.println("Linha com formato inválido: " + line);
                    }
                }
            }
            file.close();
        } else {
            Serial.println("Erro ao abrir o arquivo " + fileName);
        }
    }

    Serial.println("-------------------------------------Leitura dos agendamentos concluída.");
}


void manipularPino(int tomada, int valor) {
    Serial.print("Dentro da void manipulaPino ");
    Serial.println(tomada, valor);
    digitalWrite(ledPins[tomada], valor);
    ledStatus[tomada] = (valor == HIGH);
    writeLedStateToFile(ledStatus[tomada], String("/led_state" + String(tomada + 1) + ".txt").c_str());
    Serial.println("Pino da tomada " + String(tomada + 1) + " ajustado para " + String(valor == HIGH ? "HIGH" : "LOW"));
}


void handleLigar(int tomada) {
  Serial.print("Dentro do Handler, vai ligar pino ");
  Serial.println(tomada);
  digitalWrite(ledPins[tomada], HIGH);
  ledStatus[tomada] = true;
  client.publish((String("silvanojose/tomada") + String(tomada + 1)).c_str(), "1");
  writeLedStateToFile(ledStatus[tomada], (String("/led_state") + String(tomada + 1) + ".txt").c_str());
  Serial.println("Ligando tomada " + String(tomada + 1));
}

void handleDesligar(int tomada) {
  Serial.print("Dentro do Handler, vai desligar pino ");
  Serial.println(tomada);
  digitalWrite(ledPins[tomada], LOW);
  ledStatus[tomada] = false;
  client.publish((String("silvanojose/tomada") + String(tomada + 1)).c_str(), "0");
  writeLedStateToFile(ledStatus[tomada], (String("/led_state") + String(tomada + 1) + ".txt").c_str());
  Serial.println("Desligando tomada " + String(tomada + 1));
}

void writeLedStateToFile(bool state, const char* fileName) {
  File file = SPIFFS.open(fileName, "w");
  if (!file) {
    Serial.println("Erro ao abrir o arquivo de estado do LED para escrita.");
    return;
  }
  file.print(state ? "1" : "0");
  file.close();
}

void restoreLedState() {
  for (int i = 0; i < 4; i++) {
    String fileName = String("/led_state") + String(i + 1) + ".txt";
    File file = SPIFFS.open(fileName, "r");
    if (file) {
      ledStatus[i] = file.readStringUntil('\n').toInt();
      digitalWrite(ledPins[i], ledStatus[i] ? HIGH : LOW);
      file.close();
    } else {
      Serial.println("Erro ao abrir o arquivo " + fileName);
    }
  }
}