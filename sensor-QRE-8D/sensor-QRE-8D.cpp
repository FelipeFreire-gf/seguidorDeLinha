#include <QTRSensors.h>

class SensorArray {
private:
    QTRSensors qtr;
    static const uint8_t SensorCount = 8;
    uint16_t sensorValues[SensorCount];
    uint16_t position;

public:
    SensorArray() {
        qtr.setTypeRC();
        qtr.setSensorPins((const uint8_t[]){3, 4, 5, 6, 7, 8, 9, 10}, SensorCount);
        qtr.setEmitterPin(2);
    }

    void calibrationMode() {
        digitalWrite(LED_BUILTIN, HIGH);     

        //delay(500);

        for (uint16_t i = 0; i < 400; i++) {
            qtr.calibrate();
            delay(5);
        }
        digitalWrite(LED_BUILTIN, LOW);

        Serial.print("Valores mínimos de calibração: ");
        for (uint8_t i = 0; i < SensorCount; i++) {
            Serial.print(qtr.calibrationOn.minimum[i]);
            Serial.print(' ');
        }
        Serial.println();

        Serial.print("Valores máximos de calibração: ");
        for (uint8_t i = 0; i < SensorCount; i++) {
            Serial.print(qtr.calibrationOn.maximum[i]);
            Serial.print(' ');
        }
        Serial.println("\n");
        delay(1000);
    }

    void updatePosition() {
        position = qtr.readLineBlack(sensorValues);
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

class Motors {
public:
    void controlMotors(uint16_t position) {
        if (position < 2000) {
            Serial.println("Virar à direita");
        }
        else if (position > 5000) {
            Serial.println("Virar à esquerda");
        }
        else {
            Serial.println("Seguir reto");
        }
    }
};

SensorArray sensorQTR;
Motors motors;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(9600);
    
    delay(500);
    sensorQTR.calibrationMode();
}

void loop() {
    sensorQTR.updatePosition(); 
    sensorQTR.printSensorsValues(); 
    sensorQTR.printPosition();
    motors.controlMotors(sensorQTR.getPosition()); 
    delay(250);
}
