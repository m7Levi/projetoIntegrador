#include <WiFi.h>              // Biblioteca Wi-Fi para ESP32
#include <WebServer.h>         // Biblioteca de servidor HTTP para ESP32
#include <ArduinoJson.h>       // Para criar e enviar JSON

// --- CONFIGURAÇÕES DA REDE ---
const char* ssid = "Dionizio";
const char* password = "09041907";
// -----------------------------

// Cria uma instância do servidor na porta 80 (HTTP padrão)
WebServer server(80);

// Função que será chamada quando o app acessar o endpoint "/dados"
void handleDadosSensor() {
  // Simula leituras dos sensores
  float temperatura = 27.3 + random(0, 10) / 10.0;
  float umidade = 55.8 + random(0, 10) / 10.0;
  String Namorada_mais_linda_do_mundo = "Thabatha";

  // Cria o objeto JSON de resposta
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["temperatura"] = temperatura;
  jsonDocument["umidade"] = umidade;
  jsonDocument["Namorada mais linda do mundo"] = Namorada_mais_linda_do_mundo;

  String jsonString;
  serializeJson(jsonDocument, jsonString);

  // Envia a resposta JSON
  server.send(200, "application/json", jsonString);
}

// Página inicial simples
void handleRoot() {
  server.send(200, "text/plain", "Servidor do ESP32 está no ar! Acesse /dados para ver as leituras.");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Aguarda a conexão Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.print("Endereço de IP: ");
  Serial.println(WiFi.localIP());

  // Define as rotas do servidor
  server.on("/", handleRoot);
  server.on("/dados", handleDadosSensor);

  // Inicia o servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado!");
}

void loop() {
  // Mantém o servidor ativo
  server.handleClient();
}
