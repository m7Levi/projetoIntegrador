// void emissor() {
//   Serial.println(F("Enviando sinal IR (NEC)..."));
//   irsend.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), khz);
//   delay(1000);
// }

// void receber_sinal(){
//   Serial.println(F("Pressione o botão do controle agora - apenas uma vez"));
//   delay(50);
  
//   if (x) { // se capturou algum sinal
//     digitalWrite(LEDPIN, HIGH); // LED indica que recebeu
//     Serial.println();
//     Serial.print(F("Raw: ("));
//     Serial.print((x - 1));
//     Serial.print(F(") "));
    
//     detachInterrupt(digitalPinToInterrupt(2)); // para captura durante impressão
    
//     for (int i = 1; i < x; i++) {
//       Serial.print(irBuffer[i] - irBuffer[i - 1]);
//       Serial.print(F(", "));
//     }
    
//     x = 0; // reseta para próxima captura
//     Serial.println();
//     Serial.println();
    
//     digitalWrite(LEDPIN, LOW); // apaga LED
//     attachInterrupt(digitalPinToInterrupt(2), rxIR_Interrupt_Handler, CHANGE); // reativa
//   }
// }

// void ICACHE_RAM_ATTR rxIR_Interrupt_Handler() {
//   if (x >= maxLen) return;
//   irBuffer[x++] = micros();
// }