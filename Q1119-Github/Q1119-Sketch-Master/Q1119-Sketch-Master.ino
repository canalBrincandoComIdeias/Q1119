/****************************************************************************************
  Elevador com Motor DC
  
  Desenvolvido pela Fábrica de Programas - Brincando com Ideias (www.brincandocomideias.com)
  www.youtube.com/c/BrincandoComIdeias

  Autor Flavio Guimaraes
*****************************************************************************************/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DEBUG true

#define pinMotorA 9    //Motor DC
#define pinMotorB 10   //Motor DC
#define pinBotaoA 2    //Botão - Andar 1
#define pinBotaoB 3    //Botão - Andar 2
#define pinBotaoC 4    //Botão - Andar 3
#define pinBotaoD 5    //Botão - Andar 4
#define pinSensorA 8   //Sensor de Efeito Hall - Andar 1
#define pinSensorB 11  //Sensor de Efeito Hall - Andar 2
#define pinSensorC 12  //Sensor de Efeito Hall - Andar 3
#define pinSensorD 13  //Sensor de Efeito Hall - Andar 4
#define pinBuzzer 6    //Buzzer Passivo
#define pinLedA A0     //LED - Andar 1
#define pinLedB A1     //LED - Andar 2
#define pinLedC A2     //LED - Andar 3
#define pinLedD A3     //LED - Andar 4

#define velocidadeSobe 200
#define velocidadeDesce 90

#define tempoPorta 2000
#define tempoEspera 5000

LiquidCrystal_I2C lcd(0x27, 20, 4);

String ponteiro[2] = { "v", " " };
int ledAndar[4] = { pinLedA, pinLedB, pinLedC, pinLedD };
int botaoAndar[4] = { pinBotaoA, pinBotaoB, pinBotaoC, pinBotaoD };
int enderecoAndar[4] = { 0x11, 0x12, 0x13, 0x14 };

bool estadoSensorA;
bool estadoSensorB;
bool estadoSensorC;
bool estadoSensorD;

bool estadoSensorAAnt;
bool estadoSensorBAnt;
bool estadoSensorCAnt;
bool estadoSensorDAnt;

//Variaveis públicas de execução
int acaoElevador = 0;  //0- Parado, 1-Subindo, 2-Descendo
int acaoElevadorAnt = 0;

int andar = -1;
int andarAnt = -1;
int andarDestino = -1;

//Variaveis públicas de controle da lógica do elevador
bool chamadas[3][4];  //{Cabine, Subir, Descer} por Andar

struct tipoChamada {
  int sentido;
  int andar;
};

tipoChamada chamada;
tipoChamada chamadaProxima;

int sentido;  //0- Parado, 1-Sobe, 2-Desce
int sentidoAnt;
int sentidoProxima;

int estadoPorta;  //0= Porta Aberta, 1= Porta Fechando, 2= Porta Fechada, 3= Porta Abrindo
int estadoPortaAnt;

int acao;  //0=Parado, 1=Fechando Porta, 2=Em Movimento, 3=Abrindo Porta, 4=Espera
int acaoAnt;

unsigned long delayTempo;

//Função para DEBUG
void breakPoint(String msg, int ponto = 0, bool parada = false);

//Variáveis para caracteres especiais do display
byte customChar[8][8] = { { B11110,  //0
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B11111 },
                          { B11111,  //1
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B11110 },
                          { B01111,  //2
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B11111 },
                          { B11111,  //3
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B11111,
                            B01111 },
                          { B11111,  //4
                            B11111,
                            B11111,
                            B11111,
                            B00000,
                            B00000,
                            B00000,
                            B00000 },
                          { B00000,  //5
                            B00000,
                            B00000,
                            B00000,
                            B11111,
                            B11111,
                            B11111,
                            B11111 },
                          { B00000,  //6
                            B00001,
                            B00011,
                            B00011,
                            B00111,
                            B01111,
                            B01111,
                            B11111 },
                          { B11111,  //7
                            B01111,
                            B01111,
                            B00111,
                            B00011,
                            B00011,
                            B00001,
                            B00000 } };

int numeroGrande[10][4][4] = {
  { { 2, 4, 4, 0 },  //0
    { 9, 8, 8, 9 },
    { 9, 8, 8, 9 },
    { 4, 4, 4, 4 } },
  { { 8, 8, 8, 0 },  //1
    { 8, 8, 8, 9 },
    { 8, 8, 8, 9 },
    { 8, 8, 8, 4 } },
  { { 4, 4, 4, 0 },  //2
    { 5, 5, 5, 1 },
    { 9, 8, 8, 8 },
    { 4, 4, 4, 4 } },
  { { 4, 4, 4, 0 },  //3
    { 5, 5, 5, 9 },
    { 8, 8, 8, 9 },
    { 4, 4, 4, 4 } },
  { { 2, 8, 8, 0 },  //4
    { 3, 5, 5, 9 },
    { 8, 8, 8, 9 },
    { 8, 8, 8, 4 } },
  { { 2, 4, 4, 4 },  //5
    { 3, 5, 5, 5 },
    { 8, 8, 8, 9 },
    { 4, 4, 4, 4 } },
  { { 2, 4, 4, 4 },  //6
    { 9, 5, 5, 5 },
    { 9, 8, 8, 9 },
    { 4, 4, 4, 4 } },
  { { 4, 4, 4, 0 },  //7
    { 8, 8, 8, 9 },
    { 8, 8, 8, 9 },
    { 8, 8, 8, 4 } },
  { { 2, 4, 4, 0 },  //8
    { 9, 5, 5, 9 },
    { 9, 8, 8, 9 },
    { 4, 4, 4, 4 } },
  { { 2, 4, 4, 0 },  //9
    { 3, 5, 5, 9 },
    { 8, 8, 8, 9 },
    { 4, 4, 4, 4 } }
};

void setup() {

#ifdef DEBUG
  Serial.begin(9600);
#endif

  pinMode(pinBotaoA, INPUT_PULLUP);
  pinMode(pinBotaoB, INPUT_PULLUP);
  pinMode(pinBotaoC, INPUT_PULLUP);
  pinMode(pinBotaoD, INPUT_PULLUP);

  pinMode(pinLedA, OUTPUT);
  pinMode(pinLedB, OUTPUT);
  pinMode(pinLedC, OUTPUT);
  pinMode(pinLedD, OUTPUT);

  pinMode(pinSensorA, INPUT);
  pinMode(pinSensorB, INPUT);
  pinMode(pinSensorC, INPUT);
  pinMode(pinSensorD, INPUT);

  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinMotorA, OUTPUT);
  pinMode(pinMotorB, OUTPUT);

  Wire.begin();  // Inicia o I2C como mestre

  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  for (int nL = 0; nL < 8; nL++) {
    lcd.createChar(nL, customChar[nL]);
  }

  digitalWrite(pinLedA, HIGH);
  digitalWrite(pinLedB, HIGH);
  digitalWrite(pinLedC, HIGH);
  digitalWrite(pinLedD, HIGH);

  tone(pinBuzzer, 800);
  delay(100);
  tone(pinBuzzer, 400);
  delay(100);
  tone(pinBuzzer, 800);
  delay(100);
  noTone(pinBuzzer);

  digitalWrite(pinLedA, LOW);
  digitalWrite(pinLedB, LOW);
  digitalWrite(pinLedC, LOW);
  digitalWrite(pinLedD, LOW);

  identificaAndar();

  //Atualiza o Display dos Andares e Apaga os LEDs
  for (int nL = 0; nL < 4; nL++) {
    Wire.beginTransmission(enderecoAndar[nL]);
    Wire.write((andar + 1) + '0');
    Wire.endTransmission();

    Wire.beginTransmission(enderecoAndar[nL]);
    Wire.write('S');
    Wire.endTransmission();

    Wire.beginTransmission(enderecoAndar[nL]);
    Wire.write('D');
    Wire.endTransmission();
  }

  //Inicia o valor das variaveis públicas
  chamadaProxima.sentido = -1;
  chamadaProxima.andar = -1;

  chamada.sentido = -1;
  chamada.andar = -1;

  sentido = 0;  //0- Parado, 1-Sobe, 2-Desce
  sentidoAnt = 0;
  sentidoProxima = -1;

  estadoPorta = 0;  //0= Porta Aberta, 1= Porta Fechando, 2= Porta Fechada, 3= Porta Abrindo
  estadoPortaAnt = -1;

  acao = 0;  //0=Parado, 1=Fechando Porta, 2=Em Movimento, 3=Abrindo Porta, 4=Espera
  acaoAnt = 0;

  delayTempo = 0;
}

void loop() {

  //Controle dos botões da cabine
  for (int nL = 0; nL < 4; nL++) {
    if (!digitalRead(botaoAndar[nL])) {
      chamadas[0][nL] = true;
      digitalWrite(ledAndar[nL], HIGH);
    }
  }

  //Controle das chamadas dos andares
  for (int nL = 0; nL < 4; nL++) {
    Wire.requestFrom(enderecoAndar[nL], 1);
    if (Wire.available()) {
      char command = Wire.read();
      if ((command == 'S') || (command == 'A')) {
        chamadas[1][nL] = true;
      }
      if ((command == 'D') || (command == 'A')) {
        chamadas[2][nL] = true;
      }
    }
  }

  //Controle da Matriz
  int qtdeChamadas = 0;
  for (int nL = 0; nL < 4; nL++) {
    for (int nC = 0; nC < 3; nC++) {
      if (chamadas[nC][nL]) qtdeChamadas++;
    }
  }

  if (qtdeChamadas == 0) {
    sentido = 0;
  }

  //Identifica a próxima chamada a atender
  if (qtdeChamadas > 0) {

    //breakPoint("Sentido: " + String(sentido), 1, true);

    if (sentido == 0) {
      //Se o elevador estiver parado, procura a chamada mais próxima à cabine

      chamadaProxima.andar = -1;  //procura para cima
      chamadaProxima.sentido = -1;
      for (int nL = andar; nL < 4; nL++) {
        if (chamadas[1][nL]) {
          chamadaProxima.andar = nL;
          chamadaProxima.sentido = 1;
          sentidoProxima = 1;
          break;
        } else if (chamadas[2][nL]) {
          chamadaProxima.andar = nL;
          chamadaProxima.sentido = 2;
          sentidoProxima = 1;
          break;
        } else if (chamadas[0][nL]) {
          chamadaProxima.andar = nL;
          chamadaProxima.sentido = 0;
          sentidoProxima = 1;
          break;
        }
      }

      if (chamadaProxima.andar == -1) {  //se não achou chamadas para cima, procura para baixo
        for (int nL = andar; nL > -1; nL--) {
          if (chamadas[1][nL]) {
            chamadaProxima.andar = nL;
            chamadaProxima.sentido = 1;
            sentidoProxima = 2;
            break;
          } else if (chamadas[2][nL]) {
            chamadaProxima.andar = nL;
            chamadaProxima.sentido = 2;
            sentidoProxima = 2;
            break;
          } else if (chamadas[0][nL]) {
            chamadaProxima.andar = nL;
            chamadaProxima.sentido = 0;
            sentidoProxima = 2;
            break;
          }
        }
      }
    } else if (sentido == 1) {
      //Se o elevador estiver subindo, procura a chamada mais próxima à cabine, para cima

      chamadaProxima.andar = -1;  //procura para cima, chamadas para subir
      chamadaProxima.sentido = -1;
      for (int nL = andar; nL < 4; nL++) {
        if (chamadas[1][nL]) {
          chamadaProxima.andar = nL;
          chamadaProxima.sentido = 1;
          break;
        } else if (chamadas[0][nL]) {
          chamadaProxima.andar = nL;
          chamadaProxima.sentido = 0;
          break;
        }
      }

      if (chamadaProxima.andar == -1) {  //se não achar chamadas para subir, procura para cima, chamadas para descer
        for (int nL = 3; nL > andar; nL--) {
          if (chamadas[2][nL]) {
            chamadaProxima.andar = nL;
            chamadaProxima.sentido = 2;
            break;
          }
        }
      }

    } else if (sentido == 2) {

      chamadaProxima.andar = -1;  //procura para baixo, chamadas para descer
      chamadaProxima.sentido = -1;

      for (int nL = andar; nL > -1; nL--) {
        if (chamadas[2][nL]) {
          chamadaProxima.andar = nL;
          chamadaProxima.sentido = 2;
          break;
        } else if (chamadas[0][nL]) {
          chamadaProxima.andar = nL;
          chamadaProxima.sentido = 0;
          break;
        }
      }

      if (chamadaProxima.andar == -1) {  //se não achar chamadas para descer, procura para baixo chamadas para subir
        for (int nL = 0; nL < andar; nL++) {
          if (chamadas[1][nL]) {
            chamadaProxima.andar = nL;
            chamadaProxima.sentido = 1;
            break;
          }
        }
      }
    }
  }

  if ((qtdeChamadas > 0) && (acao == 0)) {
    acao = 1;  //Fechando a porta

    chamada = chamadaProxima;
    sentido = sentidoProxima;
    estadoPorta = 1;

    delayTempo = millis();  //Contar tempo para fechar a porta
  }

  if ((qtdeChamadas > 1) && (acao > 0)) {

    //Se existem mais de uma chamada e o elevador está em movimento
    if ((chamadaProxima.andar != -1) && (chamadaProxima.sentido != -1)) {  //Se próxima chamada foi encontrada
      if (chamada.andar != chamadaProxima.andar) {                         //Se a próxima chamada é diferente da que esta sendo executada
        chamada = chamadaProxima;
      }
    }
  }

  if (acao == 1) {
    //Se porta já foi fechada
    if ((millis() - delayTempo) > tempoPorta) {
      acao = 2;  //Movimentando Elevador
      estadoPorta = 2;
      andarDestino = chamada.andar;
    }
  }

  if (acao == 2) {
    //Se chegou no andar destino
    if (andar == andarDestino) {
      digitalWrite(ledAndar[andar], LOW);  //Apaga LED do andar
      chamadas[chamada.sentido][andar] = false;

      if (chamada.sentido == 1) {
        Wire.beginTransmission(enderecoAndar[andar]);  //Apaga LED da chamada no andar
        Wire.write('S');
        Wire.endTransmission();
      }

      if (chamada.sentido == 2) {
        Wire.beginTransmission(enderecoAndar[andar]);  //Apaga LED da chamada no andar
        Wire.write('D');
        Wire.endTransmission();
      }

      acao = 3;
      estadoPorta = 3;

      delayTempo = millis();
    }
  }

  if (acao == 3) {
    //Espera a porta abrir
    if ((millis() - delayTempo) > tempoPorta) {
      acao = 4;
      estadoPorta = 0;

      delayTempo = millis();
    }
  }

  if (acao == 4) {
    //Espera no andar
    if ((millis() - delayTempo) > tempoEspera) {
      acao = 0;
      sentido = 0;
    }
  }

  //Controle das ações
  if (andar == andarDestino) acaoElevador = 0;  //Elevador precisa parar
  if (andar < andarDestino) acaoElevador = 1;   //Elevador precisa subir
  if (andar > andarDestino) acaoElevador = 2;   //Elevador precisa descer

  //Controle dos andares
  estadoSensorA = !digitalRead(pinSensorA);
  estadoSensorB = !digitalRead(pinSensorB);
  estadoSensorC = !digitalRead(pinSensorC);
  estadoSensorD = !digitalRead(pinSensorD);

  if (estadoSensorA && !estadoSensorAAnt) andar = 0;
  if (estadoSensorB && !estadoSensorBAnt) andar = 1;
  if (estadoSensorC && !estadoSensorCAnt) andar = 2;
  if (estadoSensorD && !estadoSensorDAnt) andar = 3;

  //Executa a ação de acordo com o estado do elevador e os comandos recebidos pelos botões
  if (acaoElevador == 0) {
    if (acaoElevadorAnt == 1) {  //Estava subindo. Entao freia para baixo.
      analogWrite(pinMotorA, 255);
      analogWrite(pinMotorB, 255);
      tone(pinBuzzer, 800);
      delay(40);
      noTone(pinBuzzer);
    }
    if (acaoElevadorAnt == 2) {  //Estava descendo. Entao freia para cima.
      analogWrite(pinMotorA, 0);
      analogWrite(pinMotorB, velocidadeSobe);
      tone(pinBuzzer, 800);
      delay(40);
      noTone(pinBuzzer);
    }
    analogWrite(pinMotorA, 255);
    analogWrite(pinMotorB, 255);
  }

  if (acaoElevador == 1) {  //Subir o elevador
    analogWrite(pinMotorA, 0);
    analogWrite(pinMotorB, velocidadeSobe);
  }

  if (acaoElevador == 2) {  //Descer o elevador
    analogWrite(pinMotorA, velocidadeDesce);
    analogWrite(pinMotorB, 0);
  }

  //Atualiza o Andar no Display
  if (andar != andarAnt) {
    int dezenaAndar = ((andar + 1) / 10);
    int unidadeAndar = ((andar + 1) % 10);

    for (int nL = 0; nL < 4; nL++) {
      lcd.setCursor(0, nL);
      if (dezenaAndar > 0) {
        for (int nC = 0; nC < 4; nC++) {
          if (numeroGrande[dezenaAndar][nL][nC] == 8) {
            lcd.write(' ');
          }

          if (numeroGrande[dezenaAndar][nL][nC] == 9) {
            lcd.write(255);
          }

          if (numeroGrande[dezenaAndar][nL][nC] < 8) {
            lcd.write(byte(numeroGrande[dezenaAndar][nL][nC]));
          }
        }
      }

      lcd.setCursor(5, nL);
      for (int nC = 0; nC < 4; nC++) {
        if (numeroGrande[unidadeAndar][nL][nC] == 8) {
          lcd.write(' ');
        }

        if (numeroGrande[unidadeAndar][nL][nC] == 9) {
          lcd.write(255);
        }

        if (numeroGrande[unidadeAndar][nL][nC] < 8) {
          lcd.write(byte(numeroGrande[unidadeAndar][nL][nC]));
        }
      }
    }
  }

  //Atualiza o indicador da direção no display da cabine
  if ((sentido == 0) && (sentidoAnt != 0)) {
    for (int nL = 0; nL < 3; nL++) {
      lcd.setCursor(17, nL);
      lcd.print("   ");
    }
  }

  if ((sentido == 1) && (sentidoAnt != 1)) {
    lcd.setCursor(17, 0);
    lcd.write(' ');
    lcd.write(' ');
    lcd.write(byte(6));
    lcd.setCursor(17, 1);
    lcd.write(' ');
    lcd.write(byte(6));
    lcd.write(255);
    lcd.setCursor(17, 2);
    lcd.write(byte(6));
    lcd.write(255);
    lcd.write(255);
  }

  if ((sentido == 2) && (sentidoAnt != 2)) {
    lcd.setCursor(17, 0);
    lcd.write(byte(7));
    lcd.write(255);
    lcd.write(255);
    lcd.setCursor(17, 1);
    lcd.write(' ');
    lcd.write(byte(7));
    lcd.write(255);
    lcd.setCursor(17, 2);
    lcd.write(' ');
    lcd.write(' ');
    lcd.write(byte(7));
  }

  //Atualiza o estado da porta no display
  if ((estadoPorta == 0) && (estadoPortaAnt != 0)) {
    /*lcd.setCursor(11, 0);
      for (int nL=0; nL<4; nL++) lcd.write(byte(4));
      lcd.setCursor(11, 1);
      for (int nL=0; nL<4; nL++) lcd.write(' ');
      lcd.setCursor(11, 2);
      for (int nL=0; nL<4; nL++) lcd.write(byte(4));*/
  }

  if (((estadoPorta == 1) && (estadoPortaAnt != 1)) || ((estadoPorta == 3) && (estadoPortaAnt != 3))) {
    /*lcd.setCursor(11, 0);
      lcd.write(255);
      lcd.write(byte(4));
      lcd.write(byte(4));
      lcd.write(255);

      lcd.setCursor(11, 1);
      lcd.write(255);
      lcd.write(' ');
      lcd.write(' ');
      lcd.write(255);

      lcd.setCursor(11, 2);
      for (int nL=0; nL<4; nL++) lcd.write(byte(4));*/
  }

  if ((estadoPorta == 2) && (estadoPortaAnt != 2)) {
    /*lcd.setCursor(11, 0);
      for (int nL=0; nL<4; nL++) lcd.write(255);
      lcd.setCursor(11, 1);
      for (int nL=0; nL<4; nL++) lcd.write(255);
      lcd.setCursor(11, 2);
      for (int nL=0; nL<4; nL++) lcd.write(byte(4));*/
  }

  //Atualiza o display dos andares
  if (andar != andarAnt) {
    for (int nL = 0; nL < 4; nL++) {
      Wire.beginTransmission(enderecoAndar[nL]);
      Wire.write((andar + 1) + '0');
      Wire.endTransmission();
    }
  }

  estadoSensorAAnt = estadoSensorA;
  estadoSensorBAnt = estadoSensorB;
  estadoSensorCAnt = estadoSensorC;
  estadoSensorDAnt = estadoSensorD;

  estadoPortaAnt = estadoPorta;
  acaoElevadorAnt = acaoElevador;
  andarAnt = andar;

  sentidoAnt = sentido;
  acaoAnt = acao;
}


void identificaAndar() {

  estadoSensorA = !digitalRead(pinSensorA);
  estadoSensorB = !digitalRead(pinSensorB);
  estadoSensorC = !digitalRead(pinSensorC);
  estadoSensorD = !digitalRead(pinSensorD);

  andar = -1;
  if (estadoSensorA) andar = 0;
  if (estadoSensorB) andar = 1;
  if (estadoSensorC) andar = 2;
  if (estadoSensorD) andar = 3;

  //Caso nenhum sensor detectado, desce o elevador até que um seja encontrado.
  if (andar == -1) {
    analogWrite(pinMotorA, velocidadeDesce);
    analogWrite(pinMotorB, 0);

    do {

      estadoSensorA = !digitalRead(pinSensorA);
      estadoSensorB = !digitalRead(pinSensorB);
      estadoSensorC = !digitalRead(pinSensorC);
      estadoSensorD = !digitalRead(pinSensorD);

      if (estadoSensorA) andar = 0;
      if (estadoSensorB) andar = 1;
      if (estadoSensorC) andar = 2;
      if (estadoSensorD) andar = 3;

      if (andar != -1) {
        analogWrite(pinMotorA, 0);
        analogWrite(pinMotorB, velocidadeSobe);
        tone(pinBuzzer, 800);
        delay(40);
        noTone(pinBuzzer);
        analogWrite(pinMotorA, 255);
        analogWrite(pinMotorB, 255);

        break;
      }
    } while (andar == -1);
  }

  andarDestino = andar;
}

void breakPoint(String msg, int ponto = 0, bool parada = false) {
  Serial.print(ponto);
  Serial.print(": ");
  Serial.println(msg);

  if (parada) {
    while (!Serial.available())
      ;
    Serial.read();
  }
}