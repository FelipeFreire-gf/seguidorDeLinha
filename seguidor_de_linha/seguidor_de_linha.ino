#include "motor.h"
#include "robo.h"
#include <QTRSensors.h>

#define E_PIN_IN1 18
#define E_PIN_IN2 19
#define E_CANAL1 0
#define E_CANAL2 1

#define D_PIN_IN1 21
#define D_PIN_IN2 22
#define D_CANAL1 2
#define D_CANAL2 3 

#define FREQUENCIA 5000 //5khz alteracao 5000 - 500 teste
#define RESOLUCAO 8     //8bits

Motor motorDireito(D_PIN_IN1, D_PIN_IN2, D_CANAL1, D_CANAL2, FREQUENCIA, RESOLUCAO);
Motor motorEsquerdo(E_PIN_IN1, E_PIN_IN2, E_CANAL1, E_CANAL2, FREQUENCIA, RESOLUCAO);

Robo lineFollower(&motorEsquerdo, &motorDireito);

// propriedades dos sensores de linha
#define NUM_SENSORS               8  // number of sensors used
#define NUM_SAMPLES_PER_SENSOR    4  // average 4 analog samples per sensor reading
#define EMITTER_PIN               QTR_NO_EMITTER_PIN  // emitter is controlled by digital pin 2

QTRSensorsAnalog qtra((unsigned char[]) {36, 39, 34, 35, 32, 33, 25, 26}, NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS];

 
// prorpiedades de PID
// valor ideal = 3500
const double Kp = 0.073;     // 255/3500
// const double Ki;
const double Kd = 0.0;
double erroAnterior = 0;
const int ideal = 3500;


void setup(){
  
  // inicialização do array de sensor de linha
  calibraSensorLinha();
}

void loop(){

  // posição da linha
  unsigned int position = qtra.readLine(sensorValues);

  int erroAtual = ideal - position;
  double ajuste = Kp*erroAtual + Kd*(erroAtual - erroAnterior);
  erroAnterior = erroAtual;                                                             // Registra o erro para a próxima checagem
  lineFollower.moveFoward(constrain(255 + ajuste, 0, 255), constrain(255 - ajuste, 0, 255));   // Ajuste dos motores
}

void calibraSensorLinha(){
  delay(500);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  for (int i = 0; i < 500; i++)
  {
    qtra.calibrate();           // reads all sensors 10 times at 2.5 ms per six sensors (i.e. ~25 ms per call)
  }
  digitalWrite(13, LOW);
}