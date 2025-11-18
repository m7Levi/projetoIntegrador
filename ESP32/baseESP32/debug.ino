void debug(){
  // --- Comandos via monitor serial (para debug e testes) ---
  if (Serial.available() > 0) {
    char caractereRecebido = Serial.read();

    if (caractereRecebido == '1') {
      Serial.println("aguardando sinal");
      while (receptor() == false) { if (Serial.available() && Serial.read() == '0') break; }
      Serial.println("Comando fechado");
    }

    if (caractereRecebido == '2') {
      Serial.println("enviando IR");
      emissorIR(&IrReceiver.decodedIRData);
      Serial.println("Comando fechado");
    }

    if (caractereRecebido == '3') {
      Serial.println("Modo: Debug detectar pessoas (digite '0' para sair)");
      while (true) {
        if (Serial.available() && Serial.read() == '0') break;
        detectarPessoas();
      }
      Serial.println("Comando fechado");
    }

    if (caractereRecebido == '4') {
      Serial.println("Modo: Debug temperatura (digite '0' para sair)");
      while (true) {
        if (Serial.available() && Serial.read() == '0') break;
        LeituraSensorTemperatura();
        atualizarClimaAPI();
        Serial.print("Temperatura interna: " + String(temperatura));
        Serial.print("  |  temperatura externa: " + String(temperaturaExterna));
        Serial.println("  |  comparar temperatura: " + String(compararTemperatura()));
        delay(5000);
      }
      Serial.println("Comando fechado");
    }

    if (caractereRecebido == '5') {
      Serial.println("Modo: Debug Horário (digite '0' para sair)");
      while (true) {
        if (Serial.available() && Serial.read() == '0') break;
          printLocalTime();
      }
      Serial.println("Comando fechado");
    }
  }
}
