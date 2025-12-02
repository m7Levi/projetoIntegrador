bool receptor() {
  if (IrReceiver.decode()) {    
    if (IrReceiver.irparams.rawlen < 4) {
        Serial.print(F("Ignore data with rawlen="));
        Serial.println(IrReceiver.irparams.rawlen);
        return 0;
    }
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
        Serial.println(F("Ignore repeat"));
        return 0;
    }
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
        Serial.println(F("Ignore autorepeat"));
        return 0;
    }
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_PARITY_FAILED) {
        Serial.println(F("Ignore parity error"));
        return 0;
    }
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
        Serial.println(F("Overflow occurred, raw data did not fit into " STR(RAW_BUFFER_LENGTH) " byte raw buffer"));
        return 0;
    }
    /*
     * Copy decoded data
     */
    sStoredIRData.receivedIRData = IrReceiver.decodedIRData;

    auto tProtocol = sStoredIRData.receivedIRData.protocol;
    if (tProtocol == UNKNOWN || tProtocol == PULSE_WIDTH || tProtocol == PULSE_DISTANCE) {
        // TODO: support PULSE_WIDTH and PULSE_DISTANCE with IrSender.write
        sStoredIRData.rawCodeLength = IrReceiver.irparams.rawlen - 1;
        /*
         * Store the current raw data in a dedicated array for later usage
         */
        IrReceiver.compensateAndStoreIRResultInArray(sStoredIRData.rawCode);
        /*
         * Print info
         */
        Serial.print(F("Received unknown or pulse width/distance code and store "));
        Serial.print(IrReceiver.irparams.rawlen - 1);
        Serial.println(F(" timing entries as raw in buffer of size " STR(RAW_BUFFER_LENGTH)));
        IrReceiver.printIRResultRawFormatted(&Serial, true); // Output the results in RAW format

    } else {
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRSendUsage(&Serial);
        sStoredIRData.receivedIRData.flags = 0; // clear flags -esp. repeat- for later sending
        Serial.println();
    }
    IrReceiver.resume();
    return 1;
  }
  return 0;
}

void emissor(storedIRDataStruct *aIRDataToSend) {
  IrReceiver.stop();
  auto tProtocol = aIRDataToSend->receivedIRData.protocol;
  if (tProtocol == UNKNOWN || tProtocol == PULSE_WIDTH || tProtocol == PULSE_DISTANCE /* i.e. raw */) {
      // Assume 38 KHz
      IrSender.sendRaw(aIRDataToSend->rawCode, aIRDataToSend->rawCodeLength, 38);

      Serial.print(F("raw "));
      Serial.print(aIRDataToSend->rawCodeLength);
      Serial.println(F(" marks or spaces"));
  } else {
      /*
        * Use the write function, which does the switch for different protocols
        */
      IrSender.write(&aIRDataToSend->receivedIRData);
      printIRDataShort(&Serial, &aIRDataToSend->receivedIRData);
  }
  IrReceiver.start();
}


























// bool receptor(){
//   if (IrReceiver.decode()) {
//     Serial.println(F("Sinal IR recebido!"));

//     if (IrReceiver.decodedIRData.protocol == UNKNOWN) {

//     }
//     IrReceiver.resume(); // Pronto para receber o próximo sinal
//     return 1;
//   }
//   return 0;
// }



// void emissorIR(IRData *dados) {

//   // NEC ------------------------------------------------------
//   if (dados->protocol == NEC2) {
//     IrSender.sendNEC2(dados->address, dados->command, 1);
//     Serial.println(F("NEC2 reenviado."));
//     arCondicionado = !arCondicionado;
//   }

//   // SONY -----------------------------------------------------
//   else if (dados->protocol == SONY) {
//     IrSender.sendSony(dados->address, dados->command, dados->numberOfBits, 1);
//     Serial.println(F("Sony reenviado."));
//     arCondicionado = !arCondicionado;
//   }

//   // RC5 ------------------------------------------------------
//   else if (dados->protocol == RC5) {
//     IrSender.sendRC5(dados->address, dados->command, 1);
//     Serial.println(F("RC5 reenviado."));
//     arCondicionado = !arCondicionado;
//   }

//   // RC6 ------------------------------------------------------
//   else if (dados->protocol == RC6) {
//     IrSender.sendRC6(dados->address, dados->command, 1);
//     Serial.println(F("RC6 reenviado."));
//     arCondicionado = !arCondicionado;
//   }

//   // SAMSUNG --------------------------------------------------
//   else if (dados->protocol == SAMSUNG) {
//     IrSender.sendSamsung(dados->address, dados->command, 1);
//     Serial.println(F("Samsung reenviado."));
//     arCondicionado = !arCondicionado;
//   }

//   // LG --------------------------------------------------------
//   else if (dados->protocol == LG) {
//     IrSender.sendLG(dados->address, dados->command, 1);
//     Serial.println(F("LG reenviado."));
//     arCondicionado = !arCondicionado;
//   }

//   // RAW -------------------------------------------------------
//   else if (dados->protocol == UNKNOWN) {

//   }

//   // NÃO SUPORTADO --------------------------------------------
//   else {
//     Serial.print(F("Protocolo não suportado: "));
//     Serial.println(dados->protocol);
//   }
// }
