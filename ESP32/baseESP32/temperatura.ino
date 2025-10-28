void atualizarSensorTemperatura() {
  temperatura = dht.readTemperature();

  if (isnan(temperatura)){
    Serial.println("Falha ao ler DHT11!");
    return;
  }
  // Imprime no serial para debug
  Serial.printf("Temperatura: %.1f °C | Pessoas: %d\n", temperatura, contarPessoas);
}
