#include <QTRSensors.h>
#include <OrangutanLEDs.h>
#include <OrangutanMotors.h>

OrangutanLEDs leds;
OrangutanMotors motors;

#define LED_SENSOR 2
#define LED_STATUS 8

QTRSensors qtr;

const uint8_t sensorCount = 6;
uint16_t sensorValues[sensorCount];

int dadoBluetooth = 0;
bool seguindoLinha = false;
bool modoLento = false;
bool usandoLinhaPreta = false; // true: linha preta; false: linha branca

int velocidadeLenta = 80;

// ---------- Controle PID ----------
double Kp = 0.06;
double Ki = 0.0005;
double Kd = 0.3;

int erroAnterior = 0;
long erroIntegral = 0;

void setup() {
  pinMode(LED_SENSOR, OUTPUT);
  digitalWrite(LED_SENSOR, HIGH);

  pinMode(LED_STATUS, OUTPUT);
  digitalWrite(LED_STATUS, LOW);

  qtr.setTypeRC();
  // qtr.setSensorPins((const uint8_t[]){7, A5, A4, A3, A2, A1, 8, 4}, sensorCount);
  qtr.setSensorPins((const uint8_t[]){A4, A3, A2, A1, 8, 4}, sensorCount);
  qtr.setEmitterPin(LED_SENSOR);

  motors.setSpeeds(0, 0);
  Serial.begin(9600);
  delay(100);
}

void loop() {
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim(); // remove espaços e quebras

    if (comando == "1") {
      digitalWrite(LED_STATUS, HIGH);
      motors.setSpeeds(200, 200);
      seguindoLinha = false;
      Serial.println("Motores LIGADOS");
    }
    else if (comando == "2") {
      motors.setSpeeds(0, 0);
      seguindoLinha = false;
      Serial.println("Motores PARADOS");
    }
    else if (comando == "0") {
      digitalWrite(LED_STATUS, LOW);
      qtr.read(sensorValues);
      Serial.println("Leitura dos sensores:");
      for (uint8_t i = 0; i < sensorCount; i++) {
        Serial.print("Sensor ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(sensorValues[i]);
      }
      Serial.println("----------------------");
    }
    else if (comando == "3") {
      seguindoLinha = true;
      modoLento = false;
      erroAnterior = 0;
      erroIntegral = 0;
      Serial.println("MODO: SEGUIDOR DE LINHA NORMAL ATIVADO");
    }
    else if (comando == "4") {
      calibrarSensores();
    }
    else if (comando == "5") {
      seguindoLinha = true;
      modoLento = true;
      erroAnterior = 0;
      erroIntegral = 0;
      Serial.println("MODO: SEGUIDOR DE LINHA LENTO ATIVADO");
    }
    else if (comando == "6") {
      Serial.print("Kp: ");
      Serial.println(Kp);

      Serial.print("Ki: ");
      Serial.println(Ki);

      Serial.print("Kd: ");
      Serial.println(Kd);

      Serial.print("Velocidade: ");
      Serial.println(velocidadeLenta);

      if (usandoLinhaPreta) {
        Serial.print("Modo: Seguindo linha PRETA");
        Serial.println();
      } else {
        Serial.print("Modo: Seguindo linha BRANCA");
        Serial.println();
      }
      
    }
    else if (comando.startsWith("p")) {
      Kp = comando.substring(1).toDouble();
      Serial.print("Novo Kp: ");
      Serial.println(Kp);
    }
    else if (comando.startsWith("i")) {
      Ki = comando.substring(1).toDouble();
      Serial.print("Novo Ki: ");
      Serial.println(Ki);
    }
    else if (comando.startsWith("d")) {
      Kd = comando.substring(1).toDouble();
      Serial.print("Novo Kd: ");
      Serial.println(Kd);
    }
    else if (comando == "b") {
      usandoLinhaPreta = true;
      Serial.println("Modo: seguir linha PRETA");
    }
    else if (comando == "w") {
      usandoLinhaPreta = false;
      Serial.println("Modo: seguir linha BRANCA");
    }
    else if (comando.startsWith("v")) {
    velocidadeLenta = comando.substring(1).toInt();
    Serial.print("Nova velocidade lenta: ");
    Serial.println(velocidadeLenta);
    }
  }

  if (seguindoLinha) {
    uint16_t pos = usandoLinhaPreta ? qtr.readLineBlack(sensorValues)
                                     : qtr.readLineWhite(sensorValues);

    int erro = pos - 3500;
    erroIntegral += erro;
    int derivada = erro - erroAnterior;

    int correcao = (int)(Kp * erro + Ki * erroIntegral + Kd * derivada);
    erroAnterior = erro;

    int velocidadeBase = modoLento ? velocidadeLenta : 150;

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

  for (uint16_t i = 0; i < 50; i++) {
    motors.setSpeeds(80, 80);
    qtr.calibrate();
    delay(25);
  }

  motors.setSpeeds(0, 0);
  digitalWrite(LED_STATUS, LOW);
  Serial.println("CALIBRACAO FINALIZADA!");

  Serial.println("Valores máximos:");
  for (uint8_t i = 0; i < sensorCount; i++) {
    Serial.print(qtr.calibrationOn.maximum[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.println("Valores mínimos:");
  for (uint8_t i = 0; i < sensorCount; i++) {
    Serial.print(qtr.calibrationOn.minimum[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("----------------------");
}