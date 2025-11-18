void printLocalTime(){

  lerHoras();
  // Formatos comuns:
  // Ex: 2025-10-21 14:30:15
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.println("");
  Serial.print("Hora local (Brasília): ");
  Serial.println(buf);

  // Exemplo com nome do dia e fuso
  strftime(buf2, sizeof(buf2), "%A, %d %B %Y %H:%M:%S", &timeinfo);
  Serial.println(buf2);

  Serial.println(horario(timeinfo.tm_hour, timeinfo.tm_min));
  delay(350);
 
}

String horario(char input[]){
  return  String (input);
}

int horario(int H, int M){
  return M + (H * 60);
}

bool horariosAgendados(){
  lerHoras();
  int horaExata =  horario(timeinfo.tm_hour, timeinfo.tm_min);
  if (horaExata >= horario(8,30) && horaExata <= horario(9,0)) return 1;
  else if (horaExata >= horario(12,0) && horaExata <= horario(13,0)) return 1;
  else if (horaExata >= horario(16,0) && horaExata <= horario(16,30)) return 1;
  else if (horaExata >= horario(17,30) && horaExata <= horario(19,0)) return 1;
  return 0;
}

bool horarioFechar(){
  lerHoras();
  int horaExata = horario(timeinfo.tm_hour, timeinfo.tm_min);
  return horaExata >= horario(20,58) ? 1 : 0;
}

void lerHoras(){
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter hora local");
    return;
  }
}