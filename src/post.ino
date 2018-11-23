#include <Arduino.h>


/*
funções usadas para postar e gerenciar os posts
*/
void postar() {
  // Serial.println("Entrou no postar");
  char json[150];
  char big_json[200];
  if (filaPulsos.count() <= 0 || desconectado || cont_tentativas_pulsos>maxTentativas){
    cont_tentativas_pulsos=0;
    return;
  }
  DynamicJsonBuffer jsonBuffer;
  dadosPulsos* d = filaPulsos.peek();

  JsonObject& hidro1 = jsonBuffer.createObject();
  hidro1["serial"] = uuid_dispositivo;
  hidro1["pulso"] = d->pulsos;

  JsonObject& root = jsonBuffer.createObject();
  JsonArray& data_hor = root.createNestedArray("dados");
  data_hor.add(hidro1);
   root["data_hora"] = d->data_hora;
   root.printTo(big_json);

  HTTPClient http;
  http.begin(LOGCONTAGEM);

  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(big_json);//Retorna o código http, caso não conecte irá retornar -1
  String payload = " Payload: " + String(http.getString()) + " " + " httpCode: " + String(httpCode);
  debug(payload, 6);
  http.end();
  HttpCode(httpCode);
}

void HttpCode(int httpCode) {
  dadosPulsos* d = filaPulsos.peek();
  if (httpCode == 200) {
    String mensagem = " Tamanho da fila pulsos: " + String(filaPulsos.count());
    debug(mensagem,8);
    contador = contador -  d->pulsos * 2 ;//gravar contador na flash se couber
    ultimo_contador = ultimo_contador -  d->pulsos * 2 ;
    delete(filaPulsos.pop());
    Serial.println(system_get_free_heap_size());
    //postar();
    cont_tentativas_pulsos = 0;
  }
  else {

    cont_tentativas_pulsos++; //pushDebug pra dentro do if pra criar o debug só dps da terceira tentativa.

    Serial.print("cont_tentativas_pulsos: ");
    Serial.println(cont_tentativas_pulsos);

    if(cont_tentativas_pulsos > maxTentativas){
      //Serial.println("limite de tentativas atingido! ");
      String msgErro = "HTTP_CODE: " + String(httpCode);
      pushDebug(3, msgErro);

      enchendoFilaPulsos = true;
      enchendoFilaDebug = true;
      //  msgError = "Excedeu a quantidade maxima de tentativas";
      //  pushDebug(3, msgErro);
    }
  }
}

void pushDebug(int code_debug, String msg) {
  long rssi = WiFi.RSSI();

  char json[200];
  char json_big_debug[250];

  IPAddress ip = WiFi.localIP();
  ipStr = String(ip[0]) + String(".") + String(ip[1]) + String(".") + String(ip[2]) + String(".") + String(ip[3]);
  //Serial.print("Entrou no Debug");
  String jsonDebug;
  sprintf(dateBuffer, "%04u-%02u-%02u %02u:%02u:%02u", year(), month(), day(), hour(), minute(), second());
  DynamicJsonBuffer jsonBuffer;

   JsonObject& hidro1 = jsonBuffer.createObject();
   hidro1["serial"] = uuid_dispositivo;

   JsonObject& root = jsonBuffer.createObject();

   JsonArray& dados_debug = root.createNestedArray("dados");
   dados_debug.add(hidro1);
   root["data_hora"] = dateBuffer;
   root["sinal"] = rssi;
   root["codigo"] = code_debug;
   root["mensagem"]=msg;
   root["ip"]= ipStr;
   root.printTo(json_big_debug);
   filaErros.push(json_big_debug);
  if(filaErros.count()>tamanhoFila && !filaDebugCheia){
    filaDebugCheia = true;
    String mensagemdeErro = "tamanho da fila de Erros: " + String(filaErros.count());
    pushDebug(6, mensagemdeErro);
  }
  debug(json_big_debug,4);
  //  Serial.print(" - ");
  //  Serial.println(filaErros.pop());
  postDebug();
}

void postDebug() {
  if (filaErros.count() <= 0 || desconectado || cont_tentativas_erros>maxTentativas){
    cont_tentativas_erros=0;
    return;
  }

  HTTPClient http;
  http.begin(LOGERRO);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(filaErros.peek());//Retorna o código http, caso não conecte irá retornar -1
  String payload = "Payload: " + http.getString() + " " + "httpCode: " + String(httpCode);
  debug(payload, 6);
  http.end();
  if (httpCode == 200) {
    filaErros.pop();
  }
  else {

    cont_tentativas_erros++; //pushDebug pra dentro do if pra criar o debug só dps da terceira tentativa.

    Serial.print("cont_tentativas_erros: ");
    Serial.println(cont_tentativas_erros);

    if(cont_tentativas_erros > maxTentativas){
      //Serial.println("limite de tentativas atingido! ");
      String msgErro = "HTTP_CODE: " + String(httpCode);
      pushDebug(3, msgErro);

      enchendoFilaPulsos = true;
      enchendoFilaDebug = true;
      //  msgError = "Excedeu a quantidade maxima de tentativas";
      //  pushDebug(3, msgErro);
    }
}
}
