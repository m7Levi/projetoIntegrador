void handleDadosSensor() {

  // Cria o objeto JSON de resposta
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["temperatura"] = temperatura;
  jsonDocument["pessoas"] = contarPessoas;
  jsonDocument["horario"] = horario(buf);
  jsonDocument["controleAR"] = arCondicionado ? "ligado" : "desligado";


  String jsonString;
  serializeJson(jsonDocument, jsonString);

  // Envia a resposta JSON
  server.send(200, "application/json", jsonString);
}

void handleArControl() {
  
  if (server.hasArg("estado")) {
    estadoAlgoritmo = server.arg("estado");
    estadoAlgoritmo.toLowerCase(); // Garante que fique minusculo
  }

  if (server.hasArg("minutos")) {
    String minutosTexto = server.arg("minutos");
    minutos = minutosTexto.toInt(); // <--- IMPORTANTE: Converte Texto para Inteiro
  }
  
  StaticJsonDocument<200> json;
  
  // Preenche com os valores atuais das variaveis globais
  json["estado"] = estadoAlgoritmo; 
  json["minutos"] = minutos;

  String resposta;
  serializeJson(json, resposta);

  server.send(200, "application/json", resposta);
}


// --- Página inicial simples ---
void handleRoot() {
  server.send(200, "text/plain", "Servidor do ESP32 está no ar! Acesse /dados para ver as leituras.");
}

void atualizarClimaAPI() {
  WiFiClient client;
  const char* host = "api.openweathermap.org";
  int tentativas = 0;
  while (!client.connect(host, 80) && tentativas < 5) {
    tentativas++;
    delay(500);
  }
  if (tentativas == 5) {
    Serial.println("API falhou");
    return;
  }

  // Construção da URL
  String url = "/data/2.5/weather?q=" + String(cidade) + "," + String(pais) +
               "&appid=" + apiKey + "&units=metric";

  // Monta o request HTTP
  client.println("GET " + url + " HTTP/1.1");
  client.println("Host: api.openweathermap.org");
  client.println("Connection: close");
  client.println();

  // Recebe a resposta da API
  String payload = "";
  while (client.connected() || client.available()) {
    if (client.available()) {
      payload += client.readString();
    }
  }

  client.stop();

  // Remove o cabeçalho HTTP
  int pos = payload.indexOf("\r\n\r\n");
  payload = payload.substring(pos + 4);
  // === LER JSON ===
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    return;
  }

  // Extrai dados do JSON
  temperaturaExterna = doc["main"]["temp"];
  umidadeExterna = doc["main"]["humidity"];
}