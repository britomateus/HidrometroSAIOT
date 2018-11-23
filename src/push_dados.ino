#include <Arduino.h>


void push_dados ()
{

  bool cont_impar = 0;
  int valor = 0;
  int tamanho_fila =0;
  int pulsos;
  Serial.println(system_get_free_heap_size());
  if(filaPulsos.count()==200){
    tamanho_fila = filaPulsos.count();
    for(int i=0;i<tamanho_fila/2;i++){ // compacta a fila completa
      for(int j =0;j<2;j++){
          dadosPulsos* d = filaPulsos.peek();
          pulsos = pulsos + d->pulsos;
          Serial.print("Pulsos: ");
          Serial.println(pulsos);
          if(j==0){
            strcpy(dateBuffer,d->data_hora); // coloca no novo elemento da fila a data mais antiga
          }
          delete(filaPulsos.pop());
      }
      dadosPulsos*dado = new dadosPulsos(pulsos,dateBuffer);
      filaPulsos.push(dado);
      String mensagem = "Pulsos: " + String(dado->pulsos) + " " + "Data_hora: " + String(dado->data_hora) + " Tamanho da Fila Pulsos: " + String(filaPulsos.count()) ;
      Serial.println(mensagem);
      pulsos = 0;

  }
  }
  else{
  valor = contador - ultimo_contador;
  ultimo_contador = contador;

  if (valor % 2 != 0) {
    valor--;
    ultimo_contador--;
  }
  sprintf(dateBuffer, "%04u-%02u-%02u %02u:%02u:%02u", year(), month(), day(), hour(), minute(), second());

  dadosPulsos* d = new dadosPulsos(valor/2,dateBuffer);


  filaPulsos.push(d);
//  Serial.println(String(system_get_free_heap_size()));
  if(filaPulsos.count()>tamanhoFila && !filaPulsoCheia){
  filaPulsoCheia=true;
  String mensagemError = "Tamanho da fila de pulsos: " + String(filaPulsos.count());
  pushDebug(6, mensagemError);
  }
  String mensagem = "Pulsos: " + String(d->pulsos) + " " + "Data_hora: " + String(d->data_hora) + " Tamanho da Fila Pulsos: " + String(filaPulsos.count()) ;
  debug(mensagem,3);
}
}
