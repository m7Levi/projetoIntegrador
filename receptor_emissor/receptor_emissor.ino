#define LEDPIN 13
#define maxLen 650
 
volatile unsigned int irBuffer[maxLen]; // armazena tempos
volatile unsigned int x = 0; // índice do buffer

#include <IRremote.h>
IRsend irsend; // ligar led IR no pino 3 do arduino
 
int khz = 38; // 38kHz freq do protocolo NEC
int incomingByte = 0; // variável para o dado recebido
 
//insira neste vetor o seus dados do retirados do primeiro sketch que le o codigo IR da tecla que voce escolheu.
unsigned int meuSinalIR[] = {3304, 1752, 332, 1304, 384, 1248, 420, 472, 360, 472, 336, 496, 360, 1308, 360, 472, 384, 448, 360, 1304, 360, 1280, 412, 448, 356, 1308, 384, 448, 356, 476, 332, 1332, 408, 1232, 436, 424, 384, 1216, 472, 1228, 436, 424, 360, 472, 384, 1280, 360, 476, 408, 424, 356, 1304, 384, 452, 408, 424, 356, 476, 356, 476, 384, 448, 360, 472, 384, 448, 360, 476, 356, 476, 356, 476, 356, 476, 384, 448, 356, 476, 380, 452, 384, 448, 384, 448, 360, 476, 408, 424, 356, 476, 356, 476, 356, 1308, 384, 448, 360, 472, 332, 500, 336, 1328, 360, 476, 384, 448, 360, 472, 356, 476, 356, 476, 384, 448, 384, 1252, 440, 1252, 388, 1280, 384, 420, 412, 448, 384, 424, 408, 424, 408, 448, 412, 420, 384, 452, 408, 424, 384, 1280, 412, 1224, 436, 1256, 412, 420, 412, 420, 412, 396, 412, 444, 388, 448, 408, 396, 436, 424, 384, 448, 384, 448, 384, 448, 384, 448, 384, 448, 388, 444, 388, 444, 412, 420, 388, 448, 384, 448, 384, 448, 384, 448, 412, 420, 384, 448, 384, 448, 384, 448, 412, 420, 412, 420, 388, 444, 412, 420, 388, 444, 412, 420, 388, 448, 412, 420, 412, 420, 408, 424, 412, 420, 360, 472, 412, 1252, 412, 1252, 416, 416, 412, 1252, 416, 1248, 416, 1248, 416, 420, 412,
};
 
 
void setup() {
  pinMode(LEDPIN, OUTPUT); // importante no Uno!
  Serial.begin(115200); // Baud rate
  attachInterrupt(digitalPinToInterrupt(2), rxIR_Interrupt_Handler, CHANGE); 
  // No Uno, INT0 está no pino digital 2
}
 
void loop() {
  if (Serial.available() > 0){
    incomingByte = Serial.read();
    if (incomingByte == 'e'){
      while(incomingByte != 's'){
      
      }
    }
    if (incomingByte == 'r'){
      while(incomingByte != 's'){
      
      }
    }
  }
  incomingByte = 0;
}

void receber_sinal(){
  Serial.println(F("Pressione o botão do controle agora - apenas uma vez"));
  delay(500);
  
  if (x) { // se capturou algum sinal
    digitalWrite(LEDPIN, HIGH); // LED indica que recebeu
    Serial.println();
    Serial.print(F("Raw: ("));
    Serial.print((x - 1));
    Serial.print(F(") "));
    
    detachInterrupt(digitalPinToInterrupt(2)); // para captura durante impressão
    
    for (int i = 1; i < x; i++) {
      Serial.print(irBuffer[i] - irBuffer[i - 1]);
      Serial.print(F(", "));
    }
    
    x = 0; // reseta para próxima captura
    Serial.println();
    Serial.println();
    
    digitalWrite(LEDPIN, LOW); // apaga LED
    attachInterrupt(digitalPinToInterrupt(2), rxIR_Interrupt_Handler, CHANGE); // reativa
  }
}


void rxIR_Interrupt_Handler() {
  if (x > maxLen) return; // ignora se buffer cheio
  irBuffer[x++] = micros(); // grava timestamp da borda
}


void emitir_sinal(){
  // apenas responde quando dados são recebidos:
  if (Serial.available() > 0) {
    // lê do buffer o dado recebido:
    incomingByte = Serial.read();
  }
   
  if(incomingByte== '1') { // 1
    // responde com o dado recebido:
    Serial.println("Recebi o comando, enviando sinal");
    irsend.sendRaw(meuSinalIR, sizeof(meuSinalIR) / sizeof(meuSinalIR[0]), khz);
  }
  incomingByte=0;
}