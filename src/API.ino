#include <Arduino.h>
void dataHora(){
  /*
  Retorna a hora do sistema do ESP
  */

  String html = "<html><head><title>";
  html += uuid_dispositivo;
  html += "</title>";
  html += "<style>body { background-color: #cccccc; ";
  html += "font-family: Arial, Helvetica, Sans-Serif; ";
  html += "Color: #005356; }</style>";
  html += "</head><body>";
  html += "<h1>HORA DO ";
  html += uuid_dispositivo;
  html += "</h1>";
  html += "<svg style=\"transition-duration:150ms;position:fixed;width:100px;height:100px;margin:calc(50vh - 50px) calc(50vw - 50px)\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" id=\"Capa_1\" x=\"0px\" y=\"0px\" viewBox=\"0 0 492 492\" style=\"enable-background:new 0 0 492 492\" xml:space=\"preserve\" width=\"512px\" height=\"512px\"><g><g><polygon points=\"73,144 136,144 135,181 73,236 \" fill=\"#005356\"/><polygon points=\"245,124 0,340 26,369 246,175 466,369 492,340 247,124 246,123 \" fill=\"#005356\"/></g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g><g></g></svg>";
  html += year();
  html += "/";
  html += month();
  html += "/";
  html += day();
  html += " ";
  html += hour();
  html += ":";
  html += minute();
  html += ":";
  html += second();
  html += "</body></html>";
  // Enviando HTML para o servidor
  server.send(200, "text/html", html);
}

void dataHoraSinc(){
  /*
  sincroniza e retorna a hora do sistema do ESP
  */
  sincronizarHora();
  dataHora();
}

void estado(){
  /*
  Retorna o status do sistema
  data-hora
  janela de amostragem
  tempo de postar
  tempo de sincronizarHora
  */
}

void reinicio(/* arguments */) {
  /*
  reinicia o ESP
  */
  //ESP.
}

void configuracao(/* arguments */) {
  /*
  seta parametros de tempo
  */
}
