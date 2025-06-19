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

bool seguindoLinha = false;
bool modoLento = false;
bool usandoLinhaPreta = false;

int velocidadeLenta = 50; // Velocidade base para o modo lento
int centro = 2500;  // Posição central corrigida dos sensores (0-5000)

double Kp = 0.04;
double Ki = 0.0005;
double Kd = 0.5;

int erroAnterior = 0;
long erroIntegral = 0; 

void setup() {
  pinMode(LED_SENSOR, OUTPUT);
  digitalWrite(LED_SENSOR, HIGH); 

  pinMode(LED_STATUS, OUTPUT);
  digitalWrite(LED_STATUS, LOW); 

  qtr.setTypeRC(); 
  qtr.setSensorPins((const uint8_t[]){A4, A3, A2, A1, 8, 4}, sensorCount); // Sensores de 1 a 6
  // qtr.setSensorPins((const uint8_t[]){8, A1, A2, A3, A4, A5}, sensorCount); // Sensores de 2 a 7

  qtr.setEmitterPin(LED_SENSOR;

  motors.setSpeeds(0, 0); 
  Serial.begin(9600); 
  delay(100); 
}

void loop() {
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

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
      // Calcula a posição da linha (0 a 5000)
      uint16_t pos = usandoLinhaPreta ? qtr.readLineBlack(sensorValues)
                                      : qtr.readLineWhite(sensorValues);
      Serial.print("Posição: ");
      Serial.println(pos);
      Serial.println("----------------------");
    }
    else if (comando == "3") {
      seguindoLinha = true;
      modoLento = false; 
      erroAnterior = 0; 
      erroIntegral = 0; 
      Serial.println("MODO: SEGUIDOR DE LINHA NORMAL ATIVADO");
    }
    else if (comando == "5") {
      seguindoLinha = true;
      modoLento = true; 
      erroAnterior = 0; 
      erroIntegral = 0; 
      Serial.println("MODO: SEGUIDOR DE LINHA LENTO ATIVADO");
    }
    else if (comando == "4") {
      calibrarSensores(); 
    }
    else if (comando == "6") { // Mostra os parâmetros atuais
      Serial.print("Kp: "); Serial.println(Kp);
      Serial.print("Ki: "); Serial.println(Ki);
      Serial.print("Kd: "); Serial.println(Kd);
      Serial.print("Velocidade: "); Serial.println(velocidadeLenta);
      Serial.print("Centro: "); Serial.println(centro);
      Serial.print("Modo: Seguindo linha ");
      Serial.println(usandoLinhaPreta ? "PRETA" : "BRANCA");
    }
    else if (comando.startsWith("p")) { // Ajusta Kp
      Kp = comando.substring(1).toFloat();
      Serial.print("Novo Kp: "); Serial.println(Kp);
    }
    else if (comando.startsWith("i")) { // Ajusta Ki
      Ki = comando.substring(1).toFloat();
      Serial.print("Novo Ki: "); Serial.println(Ki);
    }
    else if (comando.startsWith("d")) { // Ajusta Kd
      Kd = comando.substring(1).toFloat();
      Serial.print("Novo Kd: "); Serial.println(Kd);
    }
    else if (comando.startsWith("v")) { // Ajusta velocidadeLenta
      velocidadeLenta = comando.substring(1).toInt();
      Serial.print("Nova velocidade lenta: "); Serial.println(velocidadeLenta);
    }
    else if (comando.startsWith("c")) { // Ajusta centro
      centro = comando.substring(1).toInt();
      Serial.print("Novo centro: "); Serial.println(centro);
    }
    else if (comando == "b") { // Muda para seguir linha PRETA
      usandoLinhaPreta = true;
      Serial.println("Modo: seguir linha PRETA");
    }
    else if (comando == "w") { // Muda para seguir linha BRANCA
      usandoLinhaPreta = false;
      Serial.println("Modo: seguir linha BRANCA");
    }
  }

  // --- MODO PID: Seguir Linha ---
  if (seguindoLinha) {
    uint16_t pos = usandoLinhaPreta ? qtr.readLineBlack(sensorValues)
                                     : qtr.readLineWhite(sensorValues);

    int erro = pos - centro; 

    erroIntegral += erro;
    // Limita o erro integral para evitar "wind-up" (acúmulo excessivo)
    erroIntegral = constrain(erroIntegral, -10000, 10000); // Pode ajustar estes limites
    int derivada = erro - erroAnterior;
    erroAnterior = erro; 

    int correcao = (int)(Kp * erro + Ki * erroIntegral + Kd * derivada);

    int velocidadeBase = modoLento ? velocidadeLenta : 150; // Use 150 para a velocidade normal, ajuste se precisar.

    // Reduz a velocidade quando o robô está em uma curva acentuada ou quase perdendo a linha
    int absErro = abs(erro); // Pega o valor absoluto do erro
    // Se o erro for significativo (ex: > 1500), reduz a velocidade
    // O valor '1500' é um ponto de partida, precisa ser ajustado nos seus testes!
    // Quanto maior o 'absErro', maior a redução da velocidade.
    if (absErro > 1500) {
      // Garante que a velocidade não caia abaixo de um mínimo (ex: 60)
      velocidadeBase = constrain(velocidadeBase - (absErro / 10), 60, velocidadeBase);
    }

    int velEsq = velocidadeBase + correcao;
    int velDir = velocidadeBase - correcao;

    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);

    motors.setSpeeds(velEsq, velDir);

void calibrarSensores() {
  // Calibração manual
  Serial.println("INICIANDO CALIBRACAO...");
  digitalWrite(LED_STATUS, HIGH); 
  delay(500); 

  for (uint16_t i = 0; i < 100; i++) { 
    // motors.setSpeeds(80, 80); 
    qtr.calibrate(); 
    delay(25); 
  }

  motors.setSpeeds(0, 0); 
  digitalWrite(LED_STATUS, LOW); 
  Serial.println("CALIBRACAO FINALIZADA!");

  Serial.println("Valores máximos:");
  for (uint8_t i = 0; i < sensorCount; i++) {
    Serial.print(qtr.calibrationOn.maximum[i]); Serial.print(" ");
  }
  Serial.println();

  Serial.println("Valores mínimos:");
  for (uint8_t i = 0; i < sensorCount; i++) {
    Serial.print(qtr.calibrationOn.minimum[i]); Serial.print(" ");
  }
  Serial.println();
  Serial.println("----------------------");
}