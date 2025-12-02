void algoritmo(){
  LeituraSensorTemperatura();
  detectarPessoas();
  if (estadoAlgoritmo == "on"){
    if (horariosAgendados() == 0){
      if (contarPessoas < 1){
        desligarArCondicionado();
      }
    }
  }
}

void desligarArCondicionado(){
  if (verificarTemperatura(10) == 1){
    if (compararTemperatura() == 1){
        emissor(&sStoredIRData);
    }
  }
}

bool compararTemperatura(){
  atualizarClimaAPI();
  if (temperatura < temperaturaExterna){
    return 1;
  }
  return 0;
}