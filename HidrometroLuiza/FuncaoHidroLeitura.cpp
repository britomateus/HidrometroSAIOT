#include <Arduino.h>
void hidro_leitura(){
  if (state ^ digitalRead(pinContPulso)) {
      state = !state;
      if(state != 0)
          {

              IntervaloHigh=micros()-UltimoTempo;
              if(IntervaloLow>=TempoMinimo)
              {
                  High=1;
              }
              }
      else
          {
              IntervaloLow=micros()-UltimoTempo;
              if(IntervaloLow>=9*TempoMinimo)
              {
                  Low=1;
              }
          }
      if(High && Low)
          {
              Pulsos=Pulsos+2;
              High=0;
              Low=0;
          }
          UltimoTempo=micros();
}
  else {
     mensagem = "Filtro de interferencia";
}
}
