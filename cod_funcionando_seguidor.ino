#include <QTRSensors.h>
#include <OrangutanLEDs.h>
#include <OrangutanMotors.h>

OrangutanLEDs leds;
OrangutanMotors motors;

// ---------- Pinos ----------
unsigned char sensorPins[] = {7, A5, A4, A3, A2, A1, A0, 4}; // d8 d7 d6 d5 d4 d3 d2 d1 ou inverso
#define LED_SENSOR 2 // led on
#define LED_STATUS 8 // led teste

QTRSensorsRC qtr;
unsigned int sensorValues[8];
int dadoBluetooth = 0;
bool seguindoLinha = false;
bool modoLento = false; 

// ---------- Controle PID ----------
float Kp = 0.06;
float Ki = 0.0001;
float Kd = 0.3;

int erroAnterior = 0;
long erroIntegral = 0;

void setup() {
  pinMode(LED_SENSOR, OUTPUT);
  digitalWrite(LED_SENSOR, HIGH);

  pinMode(LED_STATUS, OUTPUT);
  digitalWrite(LED_STATUS, LOW);

  qtr.init(sensorPins, 8);
  motors.setSpeeds(0, 0);

  Serial.begin(9600);
  delay(100);
}

void loop() {

/*
'0'	Lê sensores
'1'	Liga motores
'2'	Para motores
'3'	Seguir linha (rápido)
'4'	Calibração
'5'	Seguir linha (lento)
*/

  if (Serial.available()) {
    dadoBluetooth = Serial.read();

    if (dadoBluetooth == '1') {
      digitalWrite(LED_STATUS, HIGH);
      motors.setSpeeds(200, 200);
      seguindoLinha = false;
      Serial.println("Motores LIGADOS");
    }

    else if (dadoBluetooth == '2') {
      motors.setSpeeds(0, 0);
      seguindoLinha = false;
      Serial.println("Motores PARADOS");
    }

    else if (dadoBluetooth == '0') {
      digitalWrite(LED_STATUS, LOW);
      qtr.read(sensorValues);
      Serial.println("Leitura dos sensores:");
      for (int i = 0; i < 8; i++) {
        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(sensorValues[i]);
      }
      Serial.println("----------------------");
    }

    else if (dadoBluetooth == '3') {
      seguindoLinha = true;
      modoLento = false; // normal
      erroAnterior = 0;
      erroIntegral = 0;
      Serial.println("MODO: SEGUIDOR DE LINHA NORMAL ATIVADO");
    }

    else if (dadoBluetooth == '4') {
      calibrarSensores();
    }

    else if (dadoBluetooth == '5') {
      seguindoLinha = true;
      modoLento = true; // lento
      erroAnterior = 0;
      erroIntegral = 0;
      Serial.println("MODO: SEGUIDOR DE LINHA LENTO ATIVADO");
    }
  }

  if (seguindoLinha) {
    int pos = qtr.readLine(sensorValues); // 0 a 7000
    int erro = pos - 3500;

    erroIntegral += erro;
    int derivada = erro - erroAnterior;

    int correcao = (int)(Kp * erro + Ki * erroIntegral + Kd * derivada);
    erroAnterior = erro;

    int velocidadeBase = modoLento ? 80 : 150; // NORMAL ou LENTO

    int velEsq = velocidadeBase + correcao;
    int velDir = velocidadeBase - correcao;

    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);

    motors.setSpeeds(velEsq, velDir);
  }
}

void calibrarSensores() {
  Serial.println("INICIANDO CALIBRACAO...");
  digitalWrite(LED_STATUS, HIGH);
  delay(500);

  for (int i = 0; i < 10; i++) {
    motors.setSpeeds(100, -100);
    qtr.calibrate();
    delay(100);

    motors.setSpeeds(-100, 100);
    qtr.calibrate();
    delay(100);
  }

  motors.setSpeeds(0, 0);
  digitalWrite(LED_STATUS, LOW);
  Serial.println("CALIBRACAO FINALIZADA!");

  Serial.println("Valores máximos:");
  for (int i = 0; i < 8; i++) {
    Serial.print(qtr.calibratedMaximumOn[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.println("Valores mínimos:");
  for (int i = 0; i < 8; i++) {
    Serial.print(qtr.calibratedMinimumOn[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("----------------------");
}
