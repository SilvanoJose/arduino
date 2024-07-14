void setup() {
  Serial.begin(115200);
  Serial2.begin(57600, SERIAL_8N1, 16, 17);

  Serial.println("Teste de loopback serial iniciado");
}

void loop() {
  // Enviar dados para a própria serial (loopback)
  Serial2.println("Teste de loopback");
  delay(100);

  // Verificar se há dados recebidos na própria serial
  if (Serial2.available()) {
    String data = Serial2.readString();
    Serial.print("Recebido no loopback: ");
    Serial.println(data);
  } else {
    Serial.println("Nenhuma resposta no loopback");
  }

  delay(1000);
}
