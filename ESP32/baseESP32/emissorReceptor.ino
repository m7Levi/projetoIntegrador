bool receptor(){
  if (IrReceiver.decode()) {
    Serial.println(F("Sinal IR recebido!"));

    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
        Serial.println(F("protocolo não identificado"));
       
        IrReceiver.printIRResultRawFormatted(&Serial, true);
        IrReceiver.resume(); 
    } else {
        IrReceiver.resume();
        IrReceiver.printIRSendUsage(&Serial);
    }
    IrReceiver.resume(); // Pronto para receber o próximo sinal
    return 1;
  }
  return 0;
}




void emissorIR(IRData *dados) {
  if (dados->protocol == NEC) {
    IrSender.sendNEC2(dados->address, dados->command, 1);
    Serial.println(F("Sinal NEC2 reenviado com sucesso."));
    arCondicionado = !arCondicionado;
  }

  else if (dados->protocol == LG) {
    IrSender.sendLG(dados->address, dados->command, 1);
    Serial.println(F("Sinal LG reenviado com sucesso."));
    arCondicionado = !arCondicionado;
  }
  else {
    Serial.println(F("Protocolo não foi reenviado."));
  }
}