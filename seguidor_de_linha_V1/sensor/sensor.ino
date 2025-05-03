#include <QTRSensors.h>
#include <OrangutanLEDs.h>
#include <OrangutanMotors.h>

// pinos usados para os sensores
#define PD4 4
#define PC0 0
#define PC1 1
#define PC2 2
#define PC3 3
#define PC4 4
#define PC5 5
#define PD7 7
#define PD2 2

// diretiva de valor PWM para o motor
#define MAX_VEL 200

OrangutanLEDs leds;
OrangutanMotors motors;

class SensorArray {
private:
    static const uint8_t SensorCount = 8;
    QTRSensorsRC qtr;
    uint16_t sensorValues[SensorCount];
    uint16_t position;

public:
    SensorArray() :
        // 2.5 ms de tempo para esperar os sensores descarregarem antes de registrar próxima leitura
        qtr((const uint8_t[]){PD4, PC0, PC1, PC2, PC3, PC4, PC5, PD7}, SensorCount, 2500, PD2) {}

    void calibrationMode() {
        digitalWrite(LED_BUILTIN, HIGH);
        for (uint16_t i = 0; i < 400; i++) {
            qtr.calibrate();
            delay(5);
        }
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
    }

    void updatePosition() {
        position = qtr.readLine(sensorValues);
    }

    uint16_t getPosition() {
        return position;
    }

    void printSensorsValues() {
        Serial.print("Valores dos sensores: ");
        for (uint8_t i = 0; i < SensorCount; i++) {
            Serial.print(sensorValues[i]);
            Serial.print('\t');
        }
    }

    void printPosition() {
        Serial.print("Posição: ");
        Serial.println(position);
    }
};

SensorArray sensorQTR;

void controlMotors(uint16_t position) {
    if (position < 3000){
      if (position < 2000) {                  // vira para a direita
        motors.setSpeeds(MAX_VEL*0.75, MAX_VEL);
      }
      else{
        motors.setSpeeds(MAX_VEL*0.90, MAX_VEL);
      }
    }
    else
      if(position > 4000){
        if (position > 5000) {             // vira para a esquerda
          motors.setSpeeds(MAX_VEL, MAX_VEL*0.75);
        }
        else{
          motors.setSpeeds(MAX_VEL, MAX_VEL*0.90);
        }
      }
    else {                                  // segue em frente
        motors.setSpeeds(MAX_VEL, MAX_VEL);
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(9600);
    delay(500);
    motors.setSpeeds(200, 200);
    leds.red(HIGH);
    delay(500);
    sensorQTR.calibrationMode();
}

void loop() {
    sensorQTR.updatePosition();
    sensorQTR.printSensorsValues();
    sensorQTR.printPosition();
    uint16_t posicao = sensorQTR.getPosition();
    controlMotors(posicao);
    delay(250);
}
