void printLocalTime(){
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter hora local");
    return;
  }

  // Formatos comuns:
  // Ex: 2025-10-21 14:30:15
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.print("Hora local (Brasília): ");
  Serial.println(buf);

  // Exemplo com nome do dia e fuso
  strftime(buf2, sizeof(buf2), "%A, %d %B %Y %H:%M:%S", &timeinfo);
  Serial.println(buf2);
 
}

String horario(char input[]){
  return  String (input);
}