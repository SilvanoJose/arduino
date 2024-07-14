#include <FS.h> // Inclua a biblioteca do sistema de arquivos SPIFFS

void setup() {
  Serial.begin(115200);
  // Montar o sistema de arquivos SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Falha ao montar o sistema de arquivos SPIFFS no primeiro if");
    SPIFFS.format();
    Serial.println("Sistema de arquivos SPIFFS formatado com sucesso!  no primeiro if");
  }
  // Formatar o sistema de arquivos SPIFFS
  Serial.println("Formatando o sistema de arquivos SPIFFS... no else");
  SPIFFS.format();
  Serial.println("Sistema de arquivos SPIFFS formatado com sucesso! no else");
}

void loop() {
  // Não há necessidade de código no loop
}
