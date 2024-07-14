#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
 
#define Rele1 0
#define Rele2 15
#define Rele3 2
#define Rele4 4
 
bool Estado_Do_Rele1 = LOW;
bool Estado_Do_Rele2 = LOW;
bool Estado_Do_Rele3 = LOW;
bool Estado_Do_Rele4 = LOW;
 
// substitua os dados entre parênteses com o nome da Rede e a senha desejados
const char *ssid = "FallsBebidas";
const char *password = "87028302";
 
WiFiServer server(80);
 
void setup() {
pinMode(Rele1, OUTPUT);
pinMode(Rele2, OUTPUT);
pinMode(Rele3, OUTPUT);
pinMode(Rele4, OUTPUT);
 
Serial.begin(115200);
Serial.println();
Serial.println("configurando...");
 
 
// Você pode remover o parâmetro de senha se quiser que
// a rede gerada seja aberta.
WiFi.softAP(ssid, password);
IPAddress myIP = WiFi.softAPIP();
Serial.print("O Endereço IP Da Rede : ");
Serial.println(myIP);
 
//inicia o webserver
server.begin();
 
Serial.println("Servidor Iniciado!!");
}
 
void loop() {
 
WiFiClient client = server.available(); // verifica e armazena se tem clientes conectados na rede
 
if (client) { // se você conseguir um cliente,
Serial.println("Novo Cliente."); // imprime uma mensagem pela porta serial
String currentLine = ""; // faz uma String para manter os dados recebidos do cliente
while (client.connected()) { // loop enquanto o cliente está conectado
if (client.available()) { // se houver bytes para ler do cliente,
char c = client.read(); // leia um byte e depois
Serial.write(c); // imprima o monitor serial
if (c == '\n') { // se o byte for um caractere de nova linha
 
 
/* se a linha atual estiver em branco, você terá dois caracteres de nova linha em uma linha.
esse é o fim da solicitação HTTP do cliente, então envie uma resposta: */
if (currentLine.length() == 0) {
/* Os cabeçalhos HTTP sempre começam com um código de resposta (por exemplo, HTTP / 1.1 200 OK)
e um tipo de conteúdo para que o cliente saiba o que está por vir e, em seguida, uma linha em branco: */
client.println("HTTP/1.1 200 OK");
client.println("Content-type:text/html");
client.println();
 
 
// configuração de estilo do site
client.print("<style type=\"text/css\"> a{margin: 50px 50px; background: #000000; color: #ffffff; text-decoration: none; padding: 1% 20% 1%; border-radius: 10px; font-size: 8.0em;}</style>");
// o conteúdo do cabeçalho HTTP
client.print("<a href=\"/T1\">Rele 1. </a> </br></br></br>");
client.print("<a href=\"/T2\">Rele 2. </a> </br></br></br>");
client.print("<a href=\"/T3\">Rele 3. </a> </br></br></br>");
client.print("<a href=\"/T4\">Rele 4. </a> </br></br></br>");
 
// A resposta HTTP termina com outra linha em branco:
client.println();
// sair do loop while:
break;
} else { // se você tiver uma nova linha, então limpe a linha atual:
currentLine = "";
}
} else if (c != '\r') { // se você tiver qualquer outra coisa além de um caractere de retorno de linha,
currentLine += c; // adicioná-lo ao final da linha atual
}
 
// Verifica se o pedido do cliente foi para atualizar algum dos reles (GET /T)
 
if (currentLine.endsWith("GET /T1")) {
 
if (Estado_Do_Rele1 == LOW) {
digitalWrite(Rele1, HIGH);
Estado_Do_Rele1 = HIGH;
} else {
digitalWrite(Rele1, LOW);
Estado_Do_Rele1 = LOW;
}
}
if (currentLine.endsWith("GET /T2")) {
 
if (Estado_Do_Rele2 == LOW) {
digitalWrite(Rele2, HIGH);
Estado_Do_Rele2 = HIGH;
} else {
digitalWrite(Rele2, LOW);
Estado_Do_Rele2 = LOW;
}
}
if (currentLine.endsWith("GET /T3")) {
 
if (Estado_Do_Rele3 == LOW) {
digitalWrite(Rele3, HIGH);
Estado_Do_Rele3 = HIGH;
} else {
digitalWrite(Rele3, LOW);
Estado_Do_Rele3 = LOW;
}
}
if (currentLine.endsWith("GET /T4")) {
 
if (Estado_Do_Rele4 == LOW) {
digitalWrite(Rele4, HIGH);
Estado_Do_Rele4 = HIGH;
} else {
digitalWrite(Rele4, LOW);
Estado_Do_Rele4 = LOW;
}
}
}
}
}
// termina a conexão com o cliente
client.stop();
Serial.println("Cliente desconectado.");
}