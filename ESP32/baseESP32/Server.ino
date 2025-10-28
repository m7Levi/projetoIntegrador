void handleDadosSensor() { // MODIFICAR PARA NOSSO PROJETO

  // Cria o objeto JSON de resposta
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["temperatura"] = temperatura;
  jsonDocument["pessoas"] = contarPessoas;
  jsonDocument["horario"] = horario(buf);
  jsonDocument["horario"] = horario(buf);

  String jsonString;
  serializeJson(jsonDocument, jsonString);

  // Envia a resposta JSON
  server.send(200, "application/json", jsonString);
}

// --- Página inicial simples ---
void handleRoot() {
  server.send(200, "text/plain", "Servidor do ESP32 está no ar! Acesse /dados para ver as leituras.");
}