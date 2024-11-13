#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_BuiltIn.h>
#include "utils.h"
#include <PubSubClient.h>
#include <Keypad.h>
#include <RTClib.h>
//Criação do objeto cliente
class cliente{
  int numeroCliente;
  int horaEntrada;
  int minutoEntrada;
  int saidaAutorizada;

  public:
//Metodos set e get
  void setnumeroCliente(int numero){
    numeroCliente = numero;
  }
  void setsaidaAutorizada(int saida){
    saidaAutorizada = saida;
  }
  void settempoEntrada(int hora, int minuto){
    horaEntrada = hora;
    minutoEntrada = minuto;
  }

  int getnumeroCliente(){
    return numeroCliente;
  }
  int getsaidaAutozizada(){
    return saidaAutorizada;
  }
  int gethoraEntrada(){
    return horaEntrada;
  }
  int getminutoEntrada(){
    return minutoEntrada;
  }
//Construtor
  void construtor(){
    numeroCliente = 0;
    saidaAutorizada = 0;
  }
};
//Relogio
RTC_DS1307 rtc;

//Variaveis para os sensores do moedario
const int moeda1 = 5;
const int moeda5 = 17;
const int moeda10 = 16;
const int moeda25 = 4;
const int moeda50 = 0;

//Variaveis para os teclados numericos
const byte linhas = 2;
const byte colunas = 3;
//Construção da matriz do teclado
const char teclasMatriz[linhas][colunas] = {
  {'1', '2', '3'},
  {'4', '5', '6'}
};
//Pinos de conecção da matriz
byte pinosLinha[linhas] = {19,18};
byte pinosColuna[colunas] = {33,25,26};
//Criação do objeto para usar o segundo nucleo
TaskHandle_t Moedario;
//Criação do objeto teclado numerico
Keypad teclado1 = Keypad(makeKeymap(teclasMatriz), pinosLinha, pinosColuna, linhas, colunas);
//botao para entrar no estacionamento
const int abrirBot = 34;
//Pinos dos motores das cancelas
const int motorEntrada = 15;
const int motorSaida = 2;
//Definição dos LCDS
LiquidCrystal_I2C lcd(0x27,16,2);
LiquidCrystal_I2C lcd2(0x23,16,2);
LiquidCrystal_I2C lcd4(0x26,16,2);
//Criação dos objetos motores
Servo Entrada;
Servo Saida;

//Variaveis para a entrada
int abrirEntrada=0;
int anguloEntrada=110;
int comandoAbrirFechar=0;
int Entradafechar=0;
//Variaveis para a função millis Entrada
unsigned long tempoMotorEntrada = 0;

const long intervaloMotorEntrada = 15;
const long intervaloAbrirEntrada = 4000;
const long intervaloFecharEntrada = 4000;

unsigned long delayAbrirEntrada = 0;
unsigned long delayFecharEntrada = 0;

unsigned long delayMoeda1 = 0;
unsigned long delayMoeda5 = 0;
unsigned long delayMoeda10 = 0;
unsigned long delayMoeda25 = 0;
unsigned long delayMoeda50 = 0;

unsigned long intervaloMoedas = 150;

//Variaveis para a saida
int abrirSaida=0;
int anguloSaida=110;
int Saidafechar=0;
//Variaveis para a função millis Saida
unsigned long tempoMotorSaida = 0;

const long intervaloMotorSaida = 15;
const long intervaloAbrirSaida = 4000;
const long intervaloFecharSaida = 4000;

unsigned long delayAbrirSaida = 0;
unsigned long delayFecharSaida = 0;


//Variaveis para caso o cliente coloque o id, ou o estacionamento chegue na lotação maxima, exista uma mensagem de erro, e logo depois ela volte para o padrao
int reiniciarEntrada=0;
int reiniciarSaida=0;

int minhaVariavelGlobal = 0;

const long intervaloReiniciarEntrada = 2000;
const long intervaloReiniciarSaida = 2000;

unsigned long tempoReiniciarEntrada = 0;
unsigned long tempoReiniciarSaida = 0;

//Vetor de clientes, com a capacidade maxima do estacionamento
cliente clientes[5];

void setup() {
  Serial.begin(115200);
  for(int x=0; x<5; x++){
    clientes[x].construtor();
  }
  lcd.begin();
  lcd2.begin();
  lcd4.begin();
  pinMode(moeda1, INPUT);
  pinMode(moeda5, INPUT);
  pinMode(moeda10, INPUT);
  pinMode(moeda25, INPUT);
  pinMode(moeda50, INPUT);
  pinMode(abrirBot, INPUT);
  pinMode(motorEntrada, OUTPUT);
  pinMode(motorSaida, OUTPUT);
  connectAWS();
  Entrada.attach(motorEntrada);
  Saida.attach(motorSaida);
  pinMode(33,OUTPUT);
  lcd.print("Aperte o botao");
  lcd2.print("Digite o seu id");
  lcd4.print("Digite o seu id");
  //Declara a função que vai usar o segundo lucleo do esp
  //xTaskCreatePinnedToCore(loop2, "buttonCheck", 4000, NULL, 0, &Moedario, 0);     
}

void loop() {
  DateTime now2 = rtc.now();
  char leitor1 = teclado1.getKey();
  unsigned long tempoAtual = millis();
  client.loop();

  //reconecção do wi-fi e do broker
  if (!client.connected()) {
    Serial.println("AWS desconectado, tentando reconectar...");
    connectAWS();
  }

  //abrir a cancela da entrada
  if(digitalRead(abrirBot) == HIGH && minhaVariavelGlobal==0 && tempoAtual - delayAbrirEntrada > intervaloAbrirEntrada){
    Serial.print(now2.hour());
    lcd.clear();
    delayAbrirEntrada = tempoAtual;
    abrirEntrada = 1;
    if(clientes[0].getnumeroCliente()==0){
      clientes[0].settempoEntrada(now2.hour(), now2.minute());
      lcd.print("Seja bem vindo!");
      lcd.setCursor(0,1);
      lcd.print("Seu id e: 1");
      clientes[0].setnumeroCliente(1);
    }
    else if(clientes[1].getnumeroCliente()==0){
      clientes[1].settempoEntrada(now2.hour(), now2.minute());
      lcd.print("Seja bem vindo!");
      lcd.setCursor(0,1);
      lcd.print("Seu id e: 2");
      clientes[1].setnumeroCliente(1);
    }
    else if(clientes[2].getnumeroCliente()==0){
      clientes[2].settempoEntrada(now2.hour(), now2.minute());
      lcd.print("Seja bem vindo!");
      lcd.setCursor(0,1);
      lcd.print("Seu id e: 3");
      clientes[2].setnumeroCliente(1);
    }
    else if(clientes[3].getnumeroCliente()==0){
      clientes[3].settempoEntrada(now2.hour(), now2.minute());
      lcd.print("Seja bem vindo!");
      lcd.setCursor(0,1);
      lcd.print("Seu id e: 4");
      clientes[3].setnumeroCliente(1);
    }
    else if(clientes[4].getnumeroCliente()==0){
      clientes[4].settempoEntrada(now2.hour(), now2.minute());
      lcd.print("Seja bem vindo!");
      lcd.setCursor(0,1);
      lcd.print("Seu id e: 5");
      clientes[4].setnumeroCliente(1);
    }
    else{
      lcd.print("O estacionamento");
      lcd.setCursor(0,1);
      lcd.print("Esta lotado!");
      abrirEntrada=0;
      reiniciarEntrada=1;
      tempoReiniciarEntrada = tempoAtual;
    }
  }
//Volta para a mensagem padrao da entrada
  if(reiniciarEntrada==1 && tempoAtual - tempoReiniciarEntrada >= intervaloReiniciarEntrada){
    lcd.clear();
    lcd.print("Aperte o botao");
    reiniciarEntrada=0;
    tempoReiniciarEntrada = tempoAtual;
  }
//Abrir a cancela da entrada
  if(abrirEntrada == 1 && tempoAtual - tempoMotorEntrada >= intervaloMotorEntrada){
    tempoMotorEntrada = tempoAtual;
    Entrada.write(anguloEntrada);
    if(anguloEntrada <180){
      anguloEntrada++;
    }
    else{
      abrirEntrada = 0;
      delayFecharEntrada = tempoAtual;
      Entradafechar=2;
    }
  }
  //fechar a cancela da entrada
  if(Entradafechar == 2 && tempoAtual - delayFecharEntrada >= intervaloFecharEntrada){
    Entradafechar=1;
    delayFecharEntrada = tempoAtual;
  }
  if(Entradafechar == 1 && tempoAtual - tempoMotorEntrada >= intervaloMotorEntrada){
    tempoMotorEntrada = tempoAtual;
    Entrada.write(anguloEntrada);
    if(anguloEntrada >110){
      anguloEntrada--;
    }
    else{
      Entradafechar = 0;
      delayFecharEntrada = tempoAtual;
      lcd.clear();
      lcd.print("Aperte o botao");
    }
  }

//Saida do estacionamento
  if(leitor1){
    lcd2.clear();
    abrirSaida=1;
    if(leitor1 == '1' && clientes[0].getnumeroCliente() == 1){
      lcd2.print("Saida Autorizada!");
      clientes[0].setnumeroCliente(0);
    }
    else if(leitor1 == '2' && clientes[1].getnumeroCliente() == 1){
      lcd2.print("Saida Autorizada!");
      clientes[1].setnumeroCliente(0);
    }
    else if(leitor1 == '3' && clientes[2].getnumeroCliente() == 1){
      lcd2.print("Saida Autorizada!");
      clientes[2].setnumeroCliente(0);
    }
    else if(leitor1 == '4' && clientes[3].getnumeroCliente() == 1){
      lcd2.print("Saida Autorizada!");
      clientes[3].setnumeroCliente(0);
    }
    else if(leitor1 == '5' && clientes[4].getnumeroCliente() == 1){
      lcd2.print("Saida Autorizada!");
      clientes[4].setnumeroCliente(0);
    }
    else{
      lcd2.print("Voce Digitou");
      lcd2.setCursor(0,1);
      lcd2.print("um id invalido!");
      abrirSaida=0;
      reiniciarSaida=1;
      tempoReiniciarSaida = tempoAtual;
    }
  }
//Voltar para a mensagem padrao da saida
  if(reiniciarSaida==1 && tempoAtual - tempoReiniciarSaida >= intervaloReiniciarSaida){
    lcd2.clear();
    lcd2.print("Digite o seu id");
    reiniciarSaida=0;
    tempoReiniciarSaida = tempoAtual;
  }
//Abrir a cancela da saida
  if(abrirSaida == 1 && tempoAtual - tempoMotorSaida >= intervaloMotorSaida){
    tempoMotorSaida = tempoAtual;
    Saida.write(anguloSaida);
    if(anguloSaida <180){
      anguloSaida++;
    }
    else{
      abrirSaida = 0;
      delayFecharSaida = tempoAtual;
      Saidafechar=2;
    }
  }
//Fechar a cancela da saida
  if(Saidafechar == 2 && tempoAtual - delayFecharSaida >= intervaloFecharSaida){
    Saidafechar=1;
    delayFecharSaida = tempoAtual;
  }
  if(Saidafechar == 1 && tempoAtual - tempoMotorSaida >= intervaloMotorSaida){
    tempoMotorSaida = tempoAtual;
    Saida.write(anguloSaida);
    if(anguloSaida >110){
      anguloSaida--;
    }
    else{
      Saidafechar = 0;
      delayFecharSaida = tempoAtual;
      lcd2.clear();
      lcd2.print("Digite o seu id");
    }
  }
}