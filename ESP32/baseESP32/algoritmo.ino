void algoritmo(){
  detectarPessoas();
  if (horariosAgendados() == 0){
     if (contarPessoas < 1){
      desligarArCondicionado();
    }
  }
}

void desligarArCondicionado(){
  if (verificarTemperatura(10) == 1){
    if (compararTemperatura() == 1){
        emissorIR(&IrReceiver.decodedIRData);
    }
  }
}

bool0compararTemperatura(){
  atualizarClimaAPI();
  LeituraSensorTemperatura();
  if (temperatura < temperaturaExterna){
    return 1;
  }
  return 0;
}