#include <QTRSensors.h>
#include <OrangutanLEDs.h>
#include <OrangutanMotors.h>
#include <ctype.h> // Para a função isdigit()

OrangutanLEDs leds; 
OrangutanMotors motors;

// ---------- Pinos ----------
unsigned char sensorPins[] = {7, A5, A4, A3, A2, A1, 8, 4}; // d8 d7 d6 d5 d4 d3 d2 d1 ou inverso
#define BRUSHED 9    // PB1
#define SEN_E A6     // ADC6
#define SEN_D A0     // PC0
#define LED_SENSOR 2 // led on 

QTRSensorsRC qtr;
unsigned int sensorValues[8];
int contadorDireita = 0;                // Conta quantas vezes leu a linha da direita
const int threshold = 500;              // Valor intermeidario em que o sensor considera como preto ou branco
unsigned long ultimoTempoLateral = 0;
const unsigned long tempoLimite = 3000; // 3 segundos sem ler linha é o limite para desligar o motor brushed
// A variável 'dadoBluetooth' será lida localmente para os comandos numéricos

bool noPercurso = false;
bool seguindoLinha = false;
bool modoLento = false; 

// ---------- Controle PID ----------
float Kp = 0.06;    // Este valor será atualizável via serial
float Ki = 0.0001;  // Este valor será atualizável via serial
float Kd = 0.3;     // Este valor será atualizável via serial

int erroAnterior = 0;
long erroIntegral = 0;

String comandoRecebidoCompleto; // Para ler os comandos de string (P, I, D, G)

void setup() {
  pinMode(LED_SENSOR, OUTPUT);      
  digitalWrite(LED_SENSOR, HIGH);   

  pinMode(SEN_E, INPUT);
  pinMode(SEN_D, INPUT);
  pinMode(BRUSHED, OUTPUT);

  qtr.init(sensorPins, 8);
  motors.setSpeeds(0, 0);

  Serial.begin(9600);
  delay(100); // Delay para a serial estabilizar
  Serial.println("Baby Orangutan Pronta. Aguardando comandos."); 
}

void loop() {
  /*
  Comandos aceitos pela serial:
  '0'         Lê valores dos sensores de linha.
  '1'         Liga os motores para frente.
  '2'         Para os motores.
  '3'         Ativa o modo seguidor de linha (rápido).
  '4'         Inicia o processo de calibração.
  '5'         Ativa o modo seguidor de linha (lento).
  'P<valor>'  Define a constante Kp (ex: P0.07 seguido de Enter/Newline).
  'I<valor>'  Define a constante Ki (ex: I0.0015 seguido de Enter/Newline).
  'D<valor>'  Define a constante Kd (ex: D0.25 seguido de Enter/Newline).
  'G'         Obtém as constantes Kp, Ki, Kd atuais (seguido de Enter/Newline).
  */

  if (Serial.available()) {
    char primeiroByte = Serial.peek(); // "Espia" o primeiro byte sem removê-lo.

    // NOVO: Verifica se é um comando para ajuste de PID (P, I, D) ou para obter constantes (G)
    if (primeiroByte == 'P' || primeiroByte == 'I' || primeiroByte == 'D' || primeiroByte == 'G') {
      comandoRecebidoCompleto = Serial.readStringUntil('\n'); // Lê a string inteira
      comandoRecebidoCompleto.trim(); // Remove espaços e newlines extras

      if (comandoRecebidoCompleto.length() > 0) {
        char identificadorComando = comandoRecebidoCompleto.charAt(0);
        String valorComandoStr = "";
        if (comandoRecebidoCompleto.length() > 1) {
          valorComandoStr = comandoRecebidoCompleto.substring(1); // Pega a parte do valor
        }

        if (identificadorComando == 'P') {
          float novoKp = valorComandoStr.toFloat();
          Kp = novoKp;
          Serial.print("Kp atualizado para: ");
          Serial.println(Kp, 4); // 4 casas decimais para Kp
        } else if (identificadorComando == 'I') {
          float novoKi = valorComandoStr.toFloat();
          Ki = novoKi;
          Serial.print("Ki atualizado para: ");
          Serial.println(Ki, 6); // 6 casas decimais para Ki (pode ser bem pequeno)
        } else if (identificadorComando == 'D') {
          float novoKd = valorComandoStr.toFloat();
          Kd = novoKd;
          Serial.print("Kd atualizado para: ");
          Serial.println(Kd, 4); // 4 casas decimais para Kd
        } else if (identificadorComando == 'G') {
          Serial.print("PID_CONSTS:"); // Prefixo para o Python identificar
          Serial.print(Kp, 4); Serial.print(",");
          Serial.print(Ki, 6); Serial.print(",");
          Serial.print(Kd, 4);
          Serial.println();
        }
      }
    } 
    
    else if (isdigit(primeiroByte)) { 
      char dadoBluetooth = Serial.read(); // Lê o byte numérico

      if (dadoBluetooth == '1') { // NÃO USAR
        //digitalWrite(LED_STATUS, HIGH); // NÃO USAR
        motors.setSpeeds(200, 200);
        seguindoLinha = false;
        Serial.println("Motores LIGADOS");
      } else if (dadoBluetooth == '2') {
        motors.setSpeeds(0, 0);
        seguindoLinha = false;
        Serial.println("Motores PARADOS");
      } else if (dadoBluetooth == '0') {
      //  digitalWrite(LED_STATUS, LOW);
        qtr.read(sensorValues);
        Serial.println("Leitura dos sensores:");
        for (int i = 0; i < 8; i++) {
          Serial.print("Sensor ");
          Serial.print(i);
          Serial.print(": ");
          Serial.println(sensorValues[i]);
        }
        Serial.println("----------------------");
      } else if (dadoBluetooth == '3') {
        seguindoLinha = true;
        modoLento = false; 
        erroAnterior = 0;
        erroIntegral = 0;
        Serial.println("MODO: SEGUIDOR DE LINHA NORMAL ATIVADO");
      } else if (dadoBluetooth == '4') {
        calibrarSensores();
      } else if (dadoBluetooth == '5') {
        seguindoLinha = true;
        modoLento = true; 
        erroAnterior = 0;
        erroIntegral = 0;
        Serial.println("MODO: SEGUIDOR DE LINHA LENTO ATIVADO");
      }
      else if (dadoBluetooth == '8'){
        int estadoDireito = analogRead(SEN_D);
        Serial.println(estadoDireito);
      }
      else if(dadoBluetooth == '9'){
        digitalWrite(BRUSHED, HIGH);
      }
      else if(dadoBluetooth == 'A'){
        digitalWrite(BRUSHED, LOW);
      }
    } else {
      // Se não for um comando conhecido, lê e descarta para limpar o buffer
      Serial.read(); 
    }
  } // Fim de if (Serial.available())


  // Lógica principal do seguidor de linha
  if (seguindoLinha) {
    int senEsquerdo = analogRead(SEN_E);
    int senDireito = analogRead(SEN_D);
    bool leEsquerda = senEsquerdo > threshold;
    bool leDireita = senDireito > threshold;
    unsigned long tempoAtual= millis();

    if (leDireita && !leEsquerda){
      if (contadorDireita == 0){
        Serial.println("Percurso iniciado");
        contadorDireita = 1;
      }
      else if(contadorDireita == 1){
        Serial.println("Percurso finalizado");
        motors.setSpeeds(0,0);
        contadorDireita = 0;
      }
    }
    if (leEsquerda){
      ultimoTempoLateral = tempoAtual;
      digitalWrite(BRUSHED, HIGH);
      //Serial.println("Linha na esquerda detectada");
    }
    else{
      if(tempoAtual - ultimoTempoLateral >= tempoLimite){
        digitalWrite(BRUSHED, LOW);
        //Serial.println("Motores brushed desligados");
      }
    }

    int pos = qtr.readLine(sensorValues); 
    //int pos = qtr.readLine(sensorValues, QTR_EMITTERS_ON, true);
    int erro = pos - 3500; // Ajuste este valor se o centro da linha for diferente (numSensores-1)*1000 / 2

    erroIntegral += erro;
    // Considere Anti-windup para o termo integral:
    // long limiteIntegral = 30000; 
    // if (erroIntegral > limiteIntegral) erroIntegral = limiteIntegral;
    // if (erroIntegral < -limiteIntegral) erroIntegral = -limiteIntegral;

    int derivada = erro - erroAnterior;
    
    float termoP_val = Kp * erro;
    float termoI_val = Ki * erroIntegral;
    float termoD_val = Kd * derivada;
    
    int correcao = (int)(termoP_val + termoI_val + termoD_val);
    
    erroAnterior = erro;

    int velocidadeBase = modoLento ? 80 : 100; // NORMAL ou LENTO

    int velEsq = velocidadeBase + correcao;
    int velDir = velocidadeBase - correcao;

    velEsq = constrain(velEsq, 0, 255); 
    velDir = constrain(velDir, 0, 255);

    motors.setSpeeds(velEsq, velDir);

    // A seção para enviar dados para o gráfico PID em tempo real BUGANDO TUDO, TESTAR DPS
    /*
    Serial.print("PID:"); 
    Serial.print(pos); Serial.print(",");
    Serial.print(erro); Serial.print(",");
    Serial.print(termoP_val, 2); Serial.print(","); 
    Serial.print(termoI_val, 2); Serial.print(",");
    Serial.print(termoD_val, 2); Serial.print(",");
    Serial.print(correcao);
    Serial.println();
    delay(100); 
    */
  }
} // Fim do loop()

// A função calibrarSensores() permanece a mesma, incluindo o controle do LED_STATUS.
// O carrinho deve ser posicionado perpendicularmente à linha para a calibração
void calibrarSensores() {
  Serial.println("INICIANDO CALIBRACAO...");
  //digitalWrite(LED_STATUS, HIGH); 
  delay(500);

  for (int i = 0; i < 3; i++) {
    motors.setSpeeds(100, 100);
    qtr.calibrate();
    delay(1000);

    motors.setSpeeds(-100, -100);
    qtr.calibrate();
    delay(1000);
  }

  motors.setSpeeds(0, 0);
 // digitalWrite(LED_STATUS, LOW); 
  Serial.println("CALIBRACAO FINALIZADA!");

  Serial.println("Valores maximos:");
  for (int i = 0; i < 8; i++) {
    Serial.print(qtr.calibratedMaximumOn[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.println("Valores minimos:");
  for (int i = 0; i < 8; i++) {
    Serial.print(qtr.calibratedMinimumOn[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("----------------------");
}