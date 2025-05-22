// Bibliotecas necessárias
#include <QTRSensors.h>
#include <OrangutanMotors.h>

// Sensores de linha e motores
QTRSensors qtr;
OrangutanMotors motors;

// Configurações dos sensores de linha
const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

// Parâmetros físicos do robô (ajuste conforme necessário)
const float L = 9.0;       // Distância entre rodas (cm)
const float r = 1.65;      // Raio da roda (cm)
const int CPR = 12;        // Pulsos por rotação (sem quadratura)
const int encoderPPR = CPR * 4;  // Pulsos por rotação com quadratura

// Controle PID de linha
float Kp_linha = 0.08;
float Ki_linha = 0.0005;
float Kd_linha = 0.2;

int erro_linha = 0;
int erroAnterior_linha = 0;
long erroIntegral_linha = 0;
const int velocidadeBase = 80;

// Controle de pose (PID angular e linear)
float Kp_pose = 0.8;
float Ki_pose = 0.0;
float Kd_pose = 0.1;

// Encoders
volatile long encoderEsq = 0;
volatile long encoderDir = 0;

// Protótipos
void lerSensores();
void controleLinha();
void controlePose();
void aplicarCinematica(float v, float omega);
void setSpeeds(float vR, float vL);
void setupEncoders();

void setup() {
  // Inicializa sensores QTR
  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){3,4,5,6,7,8,9,10}, SensorCount);
  qtr.setEmitterPin(2);

  delay(500);
  for (uint16_t i = 0; i < 400; i++) {
    qtr.calibrate();
    delay(20);
  }

  setupEncoders();
  Serial.begin(9600);
}

void loop() {
  lerSensores();
  controleLinha();
  controlePose();
}

// ---------- Funções separadas ---------- //

void lerSensores() {
  uint16_t posicao = qtr.readLineBlack(sensorValues);
  erro_linha = posicao - 3500;
}

void controleLinha() {
  erroIntegral_linha += erro_linha;
  int erroDerivada = erro_linha - erroAnterior_linha;
  erroAnterior_linha = erro_linha;

  float ajuste = Kp_linha * erro_linha + Ki_linha * erroIntegral_linha + Kd_linha * erroDerivada;
  float omega = ajuste / 100.0; // Fator de conversão

  aplicarCinematica(velocidadeBase * r / 100.0, omega); // v em cm/s
}

void controlePose() {
  // Pode ser expandido se o controle for baseado em coordenadas alvo
  // Aqui está acoplado ao erro de linha
}

void aplicarCinematica(float v, float omega) {
  float vR = (2 * v + omega * L) / (2 * r);
  float vL = (2 * v - omega * L) / (2 * r);
  setSpeeds(vR, vL);
}

void setSpeeds(float vR, float vL) {
  int pwmR = constrain(vR * 255 / 200, -255, 255);
  int pwmL = constrain(vL * 255 / 200, -255, 255);
  motors.setSpeeds(pwmL, pwmR);
}

// ---------- Leitura dos encoders ---------- //

void setupEncoders() {
  attachInterrupt(digitalPinToInterrupt(0), contarEncoderDir, CHANGE);
  attachInterrupt(digitalPinToInterrupt(1), contarEncoderEsq, CHANGE);
}

void contarEncoderDir() {
  encoderDir++;
}

void contarEncoderEsq() {
  encoderEsq++;
}
