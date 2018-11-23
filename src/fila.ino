#include <Arduino.h>

/*
* acho que aqui vcaberia a função
*/
void tiraFila() {
  // Serial.println("Entrou no tirarFila");
  //Serial.println(f.pop());
  delete(filaPulsos.pop());
  Serial.println(system_get_free_heap_size());
}
