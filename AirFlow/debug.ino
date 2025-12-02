void debug(){
  // --- Comandos via monitor serial (para debug e testes) ---
  if (Serial.available() > 0) {
    char caractereRecebido = Serial.read();

    if (caractereRecebido == '1') {
      Serial.println("aperte");
      while (receptor() == false) { if (Serial.available() && Serial.read() == '0') break; }
      Serial.println("S");
    }

    if (caractereRecebido == '2') {
      Serial.println("E");
      emissor(&sStoredIRData);
    }

    if (caractereRecebido == '3') {
      Serial.println("DP '0'S");
      while (true) {
        if (Serial.available() && Serial.read() == '0') break;
        detectarPessoas();
        Serial.print("P: ");
        Serial.println(contarPessoas);
      }
      Serial.println("S");
    }

    if (caractereRecebido == '4') {
      Serial.println("T'0'S");
      while (true) {
        if (Serial.available() && Serial.read() == '0') break;
        LeituraSensorTemperatura();
        atualizarClimaAPI();
        Serial.print("TI: " + String(temperatura));
        Serial.println("  |  TE: " + String(temperaturaExterna));
        delay(5000);
      }
      Serial.println("S");
    }

    if (caractereRecebido == '5') {
      Serial.println("H '0'S");
      while (true) {
        if (Serial.available() && Serial.read() == '0') break;
          printLocalTime();
      }
      Serial.println("S");
    }
  }
}
