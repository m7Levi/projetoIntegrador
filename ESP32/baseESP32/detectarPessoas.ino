void detectarPessoas() {
  VL53L0X_RangingMeasurementData_t measureA;
  VL53L0X_RangingMeasurementData_t measureB;

  // Variável para lembrar quem detectou primeiro
  static char primeiro = 0;  // 'A', 'B' ou 0 (nenhum)

  // Leitura dos sensores
  tcaselect(MultSensorA);
  loxA.rangingTest(&measureA, false);
  long distA = (measureA.RangeStatus != 4) ? measureA.RangeMilliMeter / 10 : -1;

  tcaselect(MultSensorB);
  loxB.rangingTest(&measureB, false);
  long distB = (measureB.RangeStatus != 4) ? measureB.RangeMilliMeter / 10 : -1;

  Serial.print(String(distA) + "    " + String(distB));

  // --- DETECÇÃO ---

  // Sensor A detecta
  if (distA > 0 && distA < LIMITE_DIST && !detectA) {
    detectA = true;
    Serial.println(" -> Sensor A detectou!");
    if (primeiro == 0) primeiro = 'A'; // marca quem detectou primeiro
  }

  // Sensor B detecta
  if (distB > 0 && distB < LIMITE_DIST && !detectB) {
    detectB = true;
    Serial.println(" -> Sensor B detectou!");
    if (primeiro == 0) primeiro = 'B'; // marca quem detectou primeiro
  }

  // --- AVALIAÇÃO DE PASSAGEM ---

  if (detectA && detectB) {
    if (primeiro == 'A') {
      contarPessoas++;
      Serial.print("ENTRADA detectada | Total: ");
      Serial.println(contarPessoas);
    } else if (primeiro == 'B') {
      contarPessoas = max(0, contarPessoas - 1);
      Serial.print("SAÍDA detectada | Total: ");
      Serial.println(contarPessoas);
    }

    // Reseta o ciclo
    detectA = false;
    detectB = false;
    primeiro = 0;
    delay(750); // evita múltiplos contadores na mesma passagem
  }

  // --- LIMPEZA CASO TRAVE ---
  static unsigned long ultimaLeitura = millis();
  if (millis() - ultimaLeitura > 500) {
    detectA = false;
    detectB = false;
    primeiro = 0;
  }
  ultimaLeitura = millis();

  Serial.print("   | Total: ");
  Serial.println(contarPessoas);
  delay(20);
}