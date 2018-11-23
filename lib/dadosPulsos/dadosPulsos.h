#ifndef DADOSPULSOS
#define DADOSPULSOS

#include <Arduino.h>

class dadosPulsos{
public:
  int pulsos;
  char  data_hora[26];
  dadosPulsos(int p, char dados[]);
};

#endif
