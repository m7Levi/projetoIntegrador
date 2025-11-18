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
  if (!server.hasArg("estado")) {
    server.send(400, "text/plain", "Erro: parametro 'estado' nao encontrado");
    return;
  }

  String estado = server.arg("estado");
  estado.toLowerCase();

  if (estado == "on" && arCondicionado == 0) {
    emissorIR(&IrReceiver.decodedIRData);
  } else if (estado == "off" && arCondicionado == 1) {
    emissorIR(&IrReceiver.decodedIRData);
  } else {
    server.send(400, "text/plain", "Erro: valor invalido (use on/off)");
    return;
  }

  StaticJsonDocument<100> json;
  json["controleAR"] = arCondicionado ? "ligado" : "desligado";
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
    Serial.println("[API] Falhou após 5 tentativas");
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

void atualizarClimaDebugAPI() {
  WiFiClient client;
  const char* host = "api.openweathermap.org";

  Serial.println("\n[API] Conectando ao servidor...");
  int tentativas = 0;
  while (!client.connect(host, 80) && tentativas < 5) {
    tentativas++;
    delay(500);
  }
  if (tentativas == 5) {
    Serial.println("[API] Falhou após 5 tentativas");
    return;
  }


  // Construção da URL
  String url = "/data/2.5/weather?q=" + String(cidade) + "," + String(pais) +
               "&appid=" + apiKey + "&units=metric";

  Serial.print("[API] GET: ");
  Serial.println(url);

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

  Serial.println("[API] Resposta JSON:");
  Serial.println(payload);

  // === LER JSON ===
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("[API] Falha no parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Extrai dados do JSON
  temperaturaExterna = doc["main"]["temp"];
  umidadeExterna = doc["main"]["humidity"];

  Serial.print("[API] Temp externa: ");
  Serial.println(temperaturaExterna);
  Serial.print("[API] Umidade externa: ");
  Serial.println(umidadeExterna);
}