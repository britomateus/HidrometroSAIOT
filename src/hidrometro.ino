#include <Arduino.h>
#include <Time.h>
#include <TimeLib.h>
#include <Ticker.h>
#include <QueueList.h>
#include <stdlib.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>// comunica com o dns (ArduinoOTA)
#include <DNSServer.h> // habilita servidor dns no esp (WiFiManager)
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <dadosPulsos.h>


//CONSTANTE A SEREM AJUSTADAS PARA CADA GRAVACAO
//********************************************************************
#define uuid_dispositivo "sayo_09"
#define tempjson 1//10min  tempo em segundos em que um json eh criado e colocado n fila
#define temppost 250// 30min tempo em segundos em que sao enviados os jsons na fila (com 80 carac cabem 231 jsons na fila)
#define temppostdebug 60*60 // tempo em segundos em que sao enviados os jsons com os erros
#define temp_push_flash 60*10 // inserir o valor do contador na flash a cada X minutos
#define maxTentativas 3 //numero de tentativas para envio de jsons antes de desistir
#define tamanhoFila 250// 1h para postar os erros  ???????????????????????????????
#define tempatualizahora 6*60*60 //6h tempo para atualzar a datahora via servidor
#define sinchora true //usada para debugar a API se false desabilita todas as funções de ajustar hora automaticamente.
#define inicial 0 // rndereco inicial para gravacao na flash
#define numero_de_posicoes 10 // quantidade de enderecos para gravar
#define tamanhodado sizeof(int)
#define qntBytes tamanhodado*numero_de_posicoes + 2*tamanhodado
// ip do raspberry do lab para teste de segurança
// para usar o servidor ect usar o endereco a baixo no lugar dos numeros
// http://api.saiot.ect.ufrn.br
//https://10.7.226.85:81
#define GETDATAHORA "http://api.saiot.ect.ufrn.br/v1/history/data-hora"
#define LOGCONTAGEM "http://api.saiot.ect.ufrn.br/v1/history/hidrometro/"
#define LOGERRO "http://api.saiot.ect.ufrn.br/v1/history/erro/"
//*********************************************************************


//Portas de saida e entrada esp
#define pinContPulso D7
#define pinStateRede D6 //led vermelho -> debug conexao rede

//variaveis para funcao de contagem de pulsos
volatile unsigned int contador = 0; // checar se coloca long
volatile unsigned int Pulsos = 0;
volatile unsigned int contador_acumulador = 0;
String mensagem = "";
//volatile unsigned long last_micros;
volatile unsigned long UltimoTempo;
volatile unsigned long IntervaloHigh;
volatile unsigned long IntervaloLow;
volatile unsigned int ultimo_contador = 0; //usado para gravar na memoria

//variaveis para uso da EEPROM
int addr = 0;


//filtro
bool state = true; //false p baixo -> pulso
bool High = true;
bool Low = true;
//float debouncing_time = 100000;//equivale 100 milisegundos(esse tempo é em micro)
float TempoMinimo = 100000;
int cont_tentativas_pulsos = 0; //contador das tentativas de envio
int cont_tentativas_erros = 0;

// variavel para funcao debug
int mododebug = 1; // 0 -  nada;

char dateBuffer[26];
char horaBuffer[12];

//flag
volatile bool post_it = false;
volatile bool flag_push_queue = false;
volatile bool flag_n_criou = false;
volatile bool flag_n_postou = false;
bool setInterrupt = true;
bool desconectado = true;
bool enchendoFilaPulsos = true;
bool enchendoFilaDebug = true;
bool sincroHora = false;
bool filaPulsoCheia = false;
bool filaDebugCheia = false;

// funcao para debugar
void debug(String msg, int nivel){
  if(mododebug != 0){
    if(mododebug == 1){
      Serial.println(msg);
    }
    else{
      if(mododebug == nivel){
        Serial.println(msg);
      }
    }
  }
}


//classe para gravar na flash de forma circular
template <typename T> class grava_flash{
public:
  grava_flash( int endGuarda, int tam, int numPosicoes);

  int  gravar(T dado);
  int endFinal(); //  checar tamanho da flash
  int pegarValor();

  void zerarEnd();
  void pegarEnderecoAtual();
  void gravarTeste(T val);

private:
  int end_inicial;
  int guarda_endAtual;
  int tamanhoDado;
  int n;
  int end_final;
  int end_atual;
  int cont;
};
// construtores da classe grava_flash
template <typename T>
grava_flash<T>::grava_flash(int endGuarda, int tam, int numPosicoes){
  guarda_endAtual = endGuarda;
  tamanhoDado = tam;
  n = numPosicoes;
  end_inicial = guarda_endAtual + tamanhoDado;
  end_final = guarda_endAtual + n*tamanhoDado + tamanhoDado;
  cont =  0;
}
template <typename T>
 int grava_flash<T>:: grava_flash::endFinal(){
  return end_final;
}
template <typename T>
 int grava_flash<T>:: grava_flash::pegarValor(){
  EEPROM.begin(qntBytes);
  EEPROM.get(end_atual, contador);
  EEPROM.commit();
  String msg = "Valor no end_atual: " + String(contador);
  debug(msg, 11);
  EEPROM.end();
}
template <typename T>
 int grava_flash<T>::gravar(T dado){
  EEPROM.begin(qntBytes);
  EEPROM.get(guarda_endAtual,end_atual);
  if(cont > n ){
    cont = 0; //se gravar na posicao atual mais que n vezes, incrementa o endereco
    end_atual = end_atual + tamanhoDado;
    EEPROM.put(guarda_endAtual, end_atual);
    EEPROM.commit();
    String msg = " Endereco atual : " + String(end_atual);
    debug(msg, 12);
  }
  cont++;

  if(end_atual > end_final){
    end_atual = guarda_endAtual + tamanhoDado;
    EEPROM.put(guarda_endAtual, end_atual);
    EEPROM.commit();
  }
  EEPROM.put(end_atual, dado);
  EEPROM.commit();
  EEPROM.end();
}
template <typename T>
 void grava_flash<T>::zerarEnd(){
 EEPROM.begin(qntBytes);
 end_atual = end_inicial;
 EEPROM.put(end_atual, ultimo_contador);
 EEPROM.commit();
 guarda_endAtual = ultimo_contador;
 EEPROM.put(guarda_endAtual,end_atual);
 EEPROM.commit();
 contador=0;
 EEPROM.end();
}
template <typename T>
 void grava_flash<T>::pegarEnderecoAtual(){
  EEPROM.begin(qntBytes);
  EEPROM.get(guarda_endAtual, end_atual);
  EEPROM.commit();
  EEPROM.end();
  String msg = "Enderco atual: " + String(end_atual);
  debug(msg, 12);
  pegarValor();
}
//para teste
template <typename T>
 void grava_flash<T>::gravarTeste(T val){
  char  cont = 0;
  EEPROM.begin(22);
  int i = EEPROM.put(18,val);
  EEPROM.commit();
  EEPROM.put(22,val);
  EEPROM.commit();
  EEPROM.get(18, cont);
  EEPROM.commit();
  Serial.println("Valor do i: " + String(i));
  Serial.print(" Valor no end 0: ");
  Serial.println(cont);
  EEPROM.get(22, cont);
  EEPROM.commit();
  EEPROM.end();

  Serial.print(" Valor no end 1: ");
  Serial.println(cont);
}

QueueList<dadosPulsos*> filaPulsos;
QueueList<String> filaErros;
QueueList<String> filaErroConexao;

//objeto da classe de gravar na flash
grava_flash <unsigned int> gravador =  grava_flash <unsigned int>(inicial,tamanhodado, numero_de_posicoes);

//INTERRUPÇÕES
Ticker t_push_pulsos;
Ticker t_postar;
Ticker erros_postar;
Ticker push_flash;
Ticker seta_hora;

//para contagem de pulsos



ESP8266WebServer server(80);
//ip to string
String ipStr;
String statusesp;
/*
                                              tttt
                                           ttt:::t
                                           t:::::t
                                           t:::::t
    ssssssssss       eeeeeeeeeeee    ttttttt:::::ttttttt    uuuuuu    uuuuuu ppppp   ppppppppp
  ss::::::::::s    ee::::::::::::ee  t:::::::::::::::::t    u::::u    u::::u p::::ppp:::::::::p
ss:::::::::::::s  e::::::eeeee:::::eet:::::::::::::::::t    u::::u    u::::u p:::::::::::::::::p
s::::::ssss:::::se::::::e     e:::::etttttt:::::::tttttt    u::::u    u::::u pp::::::ppppp::::::p
 s:::::s  ssssss e:::::::eeeee::::::e      t:::::t          u::::u    u::::u  p:::::p     p:::::p
   s::::::s      e:::::::::::::::::e       t:::::t          u::::u    u::::u  p:::::p     p:::::p
      s::::::s   e::::::eeeeeeeeeee        t:::::t          u::::u    u::::u  p:::::p     p:::::p
ssssss   s:::::s e:::::::e                 t:::::t    ttttttu:::::uuuu:::::u  p:::::p    p::::::p
s:::::ssss::::::se::::::::e                t::::::tttt:::::tu:::::::::::::::uup:::::ppppp:::::::p
s::::::::::::::s  e::::::::eeeeeeee        tt::::::::::::::t u:::::::::::::::up::::::::::::::::p
 s:::::::::::ss    ee:::::::::::::e          tt:::::::::::tt  uu::::::::uu:::up::::::::::::::pp
  sssssssssss        eeeeeeeeeeeeee            ttttttttttt      uuuuuuuu  uuuup::::::pppppppp
                                                                              p:::::p
                                                                              p:::::p
                                                                             p:::::::p
                                                                             p:::::::p
                                                                             p:::::::p
                                                                             ppppppppp*/
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D7,INPUT_PULLUP); //usado para gravação na memoria / arrumar D5 p o define
  pinMode(pinStateRede, OUTPUT);
  digitalWrite(pinStateRede, desconectado);

  //interrupcao externa do contador de pulsos
  attachInterrupt(digitalPinToInterrupt(pinContPulso), hidro_leitura, CHANGE); //REVER ISSO AQUI
  //adicionei o digitalPinToInterrupt pois n sabemos qual o numero da interrupcao da porta D3

  //interrupcao por tempo para gravar na flash
  push_flash.attach(temp_push_flash, actvate_push_flash);

  Serial.begin(115200);
  Serial.println(" ");
  //usado para gravação na memoria

  if(digitalRead(D5) == LOW){
    gravador.zerarEnd();
  }
  gravador.pegarEnderecoAtual();
  /*
  configuração do WiFiManager
  */
  WiFiManager wifiManager;
  wifiManager.autoConnect(uuid_dispositivo);
  Serial.println("conectado:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  desconectado = false;
  digitalWrite(pinStateRede, desconectado); //led vermelho debug
  Serial.println(WiFi.localIP());
  IPAddress ip = WiFi.localIP();
  ipStr = String(ip[0]) + String(".") + String(ip[1]) + String(".") + String(ip[2]) + String(".") + String(ip[3]);

  if (sinchora) {
    do{
      sincroHora = sincronizarHora();
      delay(10);//tempo necessário para aguardar o fechamento da conexão
    } while (!sincroHora);
  }
  pushDebug(1, "Reiniciando");
  setupOTA(8266, uuid_dispositivo);// função para o OTA (porta,nome_dispositvo)
  ArduinoOTA.begin();

  //CONFIGURAÇÃO DO SERVIDOR
  //********************************************************************
  // Atribuindo urls para funções
  server.on("/data-hora", dataHora);
  server.on("/data-hora/sinc", dataHoraSinc);
  server.on("/estado", estado);
  server.on("/reinicio", reinicio);
  server.on("/configuracao", configuracao);
  // Iniciando servidor
  server.begin();
  //********************************************************************

  erros_postar.attach(temppostdebug, actvate_post_debug);
  seta_hora.attach(tempatualizahora, actvate_seta_hora);


}

/*
lllllll
l:::::l
l:::::l
l:::::l
 l::::l    ooooooooooo      ooooooooooo   ppppp   ppppppppp
 l::::l  oo:::::::::::oo  oo:::::::::::oo p::::ppp:::::::::p
 l::::l o:::::::::::::::oo:::::::::::::::op:::::::::::::::::p
 l::::l o:::::ooooo:::::oo:::::ooooo:::::opp::::::ppppp::::::p
 l::::l o::::o     o::::oo::::o     o::::o p:::::p     p:::::p
 l::::l o::::o     o::::oo::::o     o::::o p:::::p     p:::::p
 l::::l o::::o     o::::oo::::o     o::::o p:::::p     p:::::p
 l::::l o::::o     o::::oo::::o     o::::o p:::::p    p::::::p
l::::::lo:::::ooooo:::::oo:::::ooooo:::::o p:::::ppppp:::::::p
l::::::lo:::::::::::::::oo:::::::::::::::o p::::::::::::::::p
l::::::l oo:::::::::::oo  oo:::::::::::oo  p::::::::::::::pp
llllllll   ooooooooooo      ooooooooooo    p::::::pppppppp
                                           p:::::p
                                           p:::::p
                                          p:::::::p
                                          p:::::::p
                                          p:::::::p
                                          ppppppppp*/
void loop() {
  server.handleClient();
  ArduinoOTA.handle();
  if (setInterrupt) {
    t_push_pulsos.attach(tempjson, actvate_flag_push_queue);
    delay(200);
    t_postar.attach(temppost, actvate_post_it);
    setInterrupt = false;
  }
  if (sinchora ) {
    if (!sincroHora) {
      sincroHora = true;
      sincronizarHora();
    }
  }
  digitalWrite(LED_BUILTIN, state); //debug contador de pulso no led interno
  if (flag_push_queue)
  {
    flag_push_queue = false;
    push_dados();
  }
  if (!enchendoFilaPulsos)
  {
    if (filaPulsos.count() > 0) {
      postar();
    }
    else {
      enchendoFilaPulsos = true;
    }
  }
  if (!enchendoFilaDebug)
  {
    if (filaErros.count() > 0) {
      postDebug();
    }
    else {
      enchendoFilaDebug = true;
    }
  }
  if(filaPulsos.count()==0){
    filaPulsoCheia=false;
  }
  if(filaErros.count()==0){
    filaDebugCheia=false;
  }
}
