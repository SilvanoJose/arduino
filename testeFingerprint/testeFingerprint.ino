HardwareSerial mySerial(2); // Utilize UART2 (GPIO16 para RX e GPIO17 para TX)

void setup() {
  Serial.begin(115200); // Comunicação serial com o computador
  mySerial.begin(57600, SERIAL_8N1, 16, 17); // Comunicação serial com o sensor biométrico

  Serial.println("Teste de comunicação serial iniciado");
  delay(1000); // Tempo para inicialização do sensor
}

void loop() {
  // Enviar comando para o sensor biométrico
  mySerial.write(0xEF); // Pacote inicial
  mySerial.write(0x01); // Endereço do sensor
  mySerial.write(0xFF); // Comando (ajuste conforme necessário)
  mySerial.write(0xFF); // Parâmetro (ajuste conforme necessário)
  mySerial.write(0xFF); // Checksum (ajuste conforme necessário)
  mySerial.write(0xFF); // Pacote final
  Serial.println("Comando enviado para o sensor");

  delay(500); // Atraso para garantir que o sensor tenha tempo de processar e responder

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
