void LeituraSensorTemperatura() {
  temperatura = dht.readTemperature();

  if (isnan(temperatura)){
    Serial.println("Falha ao ler DHT11!");
    return;
  }
}

long tempoPassado = 0;
float temperaturaPassada;
int cicloTempo = 1;


int verificarTemperatura(int minutos){
  LeituraSensorTemperatura();
  if (millis() - tempoPassado >= minutos * 60000){
    tempoPassado = millis();
    if (cicloTempo == 2){
      cicloTempo = 1;
      if (fabs(temperatura - temperaturaPassada) > 3){
        return 2;
      }
      return 1;
    }
    else temperaturaPassada = temperatura;
    cicloTempo = cicloTempo + 1;
    delay(10);
  }
  return 0;
}

