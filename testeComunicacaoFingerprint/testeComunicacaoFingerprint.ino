HardwareSerial mySerial(2); // Utilize UART2 (GPIO16 para RX e GPIO17 para TX)

void setup() {
  Serial.begin(115200); // Comunicação serial com o computador
  mySerial.begin(57600, SERIAL_8N1, 16, 17); // Comunicação serial com o sensor biométrico

  Serial.println("Teste de comunicação serial iniciado");
  delay(1000); // Tempo para inicialização do sensor
}

void loop() {
  // Enviar dados para o sensor
  mySerial.println("Teste de comunicação");
  Serial.println("Mensagem enviada para o sensor");

  delay(100); // Atraso para garantir que o sensor tenha tempo de processar e responder

  // Verificar se há dados recebidos do sensor
  if (mySerial.available()) {
    String data = mySerial.readString();
    Serial.print("Recebido do sensor: ");
    Serial.println(data);
  } else {
    Serial.println("Nenhuma resposta do sensor");
  }

  delay(1000); // Atraso para facilitar a leitura dos dados
}
