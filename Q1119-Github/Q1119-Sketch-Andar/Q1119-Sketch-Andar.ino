// Código para os Arduinos Pro Mini (Slaves)
#include <Wire.h>

#define pinLEDVd 3
#define pinLEDVm 5

#define pinSegA A3
#define pinSegB A2
#define pinSegC A0
#define pinSegD 10
#define pinSegE A1
#define pinSegF 11
#define pinSegG 12
#define pinSegDP 13

#define pinBotaoSobe 2
#define pinBotaoDesce 8

// Defina o endereço I2C deste Pro Mini
#define I2C_ADDRESS 0x14  // Andar 1: endereço 0x11, Andar 2: endereço 0x12

int portas[8] = { pinSegA, pinSegB, pinSegC, pinSegD, pinSegE, pinSegF, pinSegG, pinSegDP };

//a b c d e f g dp
bool digitos[10][8] = {
  { 1, 1, 1, 1, 1, 1, 0, 0 },  //0
  { 0, 1, 1, 0, 0, 0, 0, 0 },  //1
  { 1, 1, 0, 1, 1, 0, 1, 0 },  //2
  { 1, 1, 1, 1, 0, 0, 1, 0 },  //3
  { 0, 1, 1, 0, 0, 1, 1, 0 },  //4
  { 1, 0, 1, 1, 0, 1, 1, 0 },  //5
  { 1, 0, 1, 1, 1, 1, 1, 0 },  //6
  { 1, 1, 1, 0, 0, 0, 0, 0 },  //7
  { 1, 1, 1, 1, 1, 1, 1, 0 },  //8
  { 1, 1, 1, 1, 0, 1, 1, 0 }   //9
};

unsigned long delayInc;
int andar = 0;
bool estadoChamadaSobe = false;
bool estadoChamadaDesce = false;

void setup() {
  pinMode(pinLEDVd, OUTPUT);
  pinMode(pinLEDVm, OUTPUT);
  pinMode(pinSegA, OUTPUT);
  pinMode(pinSegB, OUTPUT);
  pinMode(pinSegC, OUTPUT);
  pinMode(pinSegD, OUTPUT);
  pinMode(pinSegE, OUTPUT);
  pinMode(pinSegF, OUTPUT);
  pinMode(pinSegG, OUTPUT);
  pinMode(pinSegDP, OUTPUT);

  pinMode(pinBotaoSobe, INPUT_PULLUP);
  pinMode(pinBotaoDesce, INPUT_PULLUP);

  Wire.begin(I2C_ADDRESS);  // Inicializa o I2C com o endereço específico
  Wire.onReceive(recebeDados);
  Wire.onRequest(enviarDados);
}

void loop() {

  //Botão "sobe" acionado
  if (!digitalRead(pinBotaoSobe)) {
    //estadoChamadaSobe = true;
  }

  //Botão "desce" acionado
  if (!digitalRead(pinBotaoDesce)) {
    estadoChamadaDesce = true;
  }

  digitalWrite(pinLEDVd, estadoChamadaSobe);
  digitalWrite(pinLEDVm, estadoChamadaDesce);
}

void recebeDados(int num_bytes) {
  char command = Wire.read();
  if ((command >= '0') && (command <= '9')) {
    andar = command - '0';
    for (int nL = 0; nL < 8; nL++) {
      digitalWrite(portas[nL], digitos[andar][nL]);
    }
  }

  if (command == 'S') {  //Considerar chamada para cima como atendida
    estadoChamadaSobe = false;
  }

  if (command == 'D') {  //Considerar chamada para baixo como atendida
    estadoChamadaDesce = false;
  }
}

void enviarDados() {
  if (estadoChamadaSobe) {
    if (estadoChamadaDesce) {
       Wire.write('A');  //Ambos botões apertados
    } else {
       Wire.write('S');  //Botão sobe apertado
    }
  } else {
    if (estadoChamadaDesce) {
       Wire.write('D');  //Botão desce apertado
    } else {
       Wire.write('N');  //Nenhum botão apertado
    }
  }
}