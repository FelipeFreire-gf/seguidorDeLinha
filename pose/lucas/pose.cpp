#include <QTRSensors.h>
#include <OrangutanMotors.h>

QTRSensors qtr;
OrangutanMotors motors;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

// Parâmetros de velocidade e PID
const int velocidadeBase = 80;

// Esse PID é apenas para desenvolver esse codigo pose 
float Kp = 0.08;
float Ki = 0.0005;
float Kd = 0.2;

int erro = 0;
int erroAnterior = 0;
long erroIntegral = 0;

void setup() {
    // Configura os sensores
    qtr.setTypeRC();
    qtr.setSensorPins((const uint8_t[]){3, 4, 5, 6, 7, 8, 9, 10}, SensorCount);
    qtr.setEmitterPin(2);

    delay(500);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // LED aceso durante a calibração

    for (uint16_t i = 0; i < 400; i++) {
      qtr.calibrate();
      delay(20);
    }

    digitalWrite(LED_BUILTIN, LOW); // LED apagado: calibração feita
    Serial.begin(9600);
    delay(1000);
}

void loop() {
    // Lê posição da linha (0 a 7000)
    uint16_t posicao = qtr.readLineBlack(sensorValues);
    erro = posicao - 3500;

    // PID
    erroIntegral += erro;
    int erroDerivada = erro - erroAnterior;
    erroAnterior = erro;

    int ajuste = Kp * erro + Ki * erroIntegral + Kd * erroDerivada;

    int velEsq = velocidadeBase + ajuste;
    int velDir = velocidadeBase - ajuste;

    // Limita velocidade entre -255 e 255
    velEsq = constrain(velEsq, -255, 255);
    velDir = constrain(velDir, -255, 255);

    // Aplica aos motores
    motors.setSpeeds(velEsq, velDir);

    /* DEBUG (opcional)
    Serial.print("Pos: ");
    Serial.print(posicao);
    Serial.print("  | Erro: ");
    Serial.print(erro);
    Serial.print("  | Ajuste: ");
    Serial.println(ajuste);
    */

    delay(10);
}