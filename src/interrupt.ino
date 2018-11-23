#include <Arduino.h>
void hidro_leitura(){
  if (state ^ digitalRead(pinContPulso)) {
      state = !state;
      if(state != 0) {
              IntervaloHigh=micros()-UltimoTempo;
              if(IntervaloLow>=TempoMinimo) {
                  High=1;
              }
      } else {
              IntervaloLow=micros()-UltimoTempo;
              if(IntervaloLow>=9*TempoMinimo) {
                  Low=1;
              }
      }
      if(High && Low) {
              Pulsos=Pulsos+2;
              High=0;
              Low=0;
          }
          UltimoTempo=micros();
  } else {
     mensagem = "Filtro de interferencia";
}
}
void actvate_flag_push_queue(){
  flag_push_queue = true;
}

void actvate_post_it(){
  //  post_it = true;
  if (WiFi.status() == WL_CONNECTED) {
    enchendoFilaPulsos = false;
    if (desconectado) {
      desconectado = false;
      pushDebug(4, "Reconectado" );
    }
  }
  else {
    if (desconectado == false) {
      pushDebug(5, "Desconectado" );
      desconectado = true;
    }
  }
  digitalWrite(pinStateRede, desconectado); // led debug rede
}

void actvate_post_debug(){
  if (WiFi.status() == WL_CONNECTED) {
    enchendoFilaDebug = false;
    if (desconectado) {
      desconectado = false;
      pushDebug(4, "Reconectado" );
    }
  }
  else {
    if (desconectado == false) {
      pushDebug(5, "Desconectado" );
      desconectado = true;
    }
  }
  digitalWrite(pinStateRede, desconectado);
}

void actvate_seta_hora(){
  sincroHora = false;
}
void actvate_push_flash(){
    gravador.gravar(contador);
}
