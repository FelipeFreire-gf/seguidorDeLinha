// Devido a interferencias no bluetooth esse código tem o proposito de
// fazer o robô calibrar os sensores e seguir linha de maneira manual 
// sem uso de bluetooth
#include <QTRSensors.h>
#include <OrangutanLEDs.h>
#include <OrangutanMotors.h>

OrangutanLEDs leds;
OrangutanMotors motors;

#define SEN_D A6                      // ADC6
#define BRUSHED 8                     // PB0
#define LOWH 10                       // PB2 (força sinal PWM low na ponte h)
#define MAX_DIREITA 4                 // Quantidade máxima de vezes que o sensor da direita lê linhas no circuito
#define LED_SENSOR 2

QTRSensors qtr;

const uint8_t sensorCount = 6;        // ALTERAR CASO ESTEJA USANDO SOMENTE 6 SENSORES
//const uint8_t sensorCount = 8;
uint16_t sensorValues[sensorCount];

// ------------ Sensores Laterais --------------
int contadorDireita = 0;              // Conta quantas vezes leu a linha da direita
const int threshold = 200;            // Valor intermediario em que o sensor considera como preto ou branco

bool seguindoLinha = false;
bool modoLento = false;
bool usandoLinhaPreta = false;
bool estadoAnteriorDireita = false;

int velocidadeLenta = 40;             // Velocidade base para o modo lento
int centro = 2500;                    // Posição central corrigida dos sensores (0-5000)


// ------------ CONSTANTES PID ------------
double Kp = 0.04;
double Ki = 0.0005;
double Kd = 0.5;

int erroAnterior = 0;
long erroIntegral = 0; 

void setup() {
  pinMode(LED_SENSOR, OUTPUT);
  digitalWrite(LED_SENSOR, HIGH); 

  pinMode(LOWH, OUTPUT);
  pinMode(SEN_D, INPUT);
  pinMode(BRUSHED, OUTPUT);

  qtr.setTypeRC();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, 7, 4}, sensorCount);
  qtr.setEmitterPin(LED_SENSOR);

  motors.setSpeeds(0, 0);
  Serial.begin(9600);
  delay(500);

  // -------- CALIBRAÇÃO AUTOMÁTICA --------
  calibrarSensores();

  // -------- ENTRAR DIRETO NO MODO SEGUIDOR DE LINHA --------
  seguindoLinha = true;
  modoLento = true;
  erroAnterior = 0;
  erroIntegral = 0;
  Serial.println("Modo seguidor de linha iniciado automaticamente!");
}

void loop() {
  digitalWrite(BRUSHED,HIGH);
  digitalWrite(LOWH,LOW);
  
  // --- MODO PID: Seguir Linha ---
  if (seguindoLinha) {
    digitalWrite(BRUSHED, HIGH);
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
    /*
      Reduz a velocidade quando o robô está em uma curva acentuada ou quase perdendo a linha
      Pega o valor absoluto do erro
      Se o erro for significativo (ex: > 1500), reduz a velocidade
      O valor '1500' é um ponto de partida, precisa ser ajustado nos seus testes!
      Quanto maior o 'absErro', maior a redução da velocidade.
    */
    int absErro = abs(erro);
    if (absErro > 1500) {
      // Garante que a velocidade não caia abaixo de um mínimo (ex: 60)
      velocidadeBase = constrain(velocidadeBase - (absErro / 10), velocidadeLenta, velocidadeBase);
    }
    int velEsq = velocidadeBase + correcao;
    int velDir = velocidadeBase - correcao;
    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);
    
    motors.setSpeeds(velEsq, velDir);

    // Leitura dos sensores laterais
    int senDireito = analogRead(SEN_D);
    bool leDireita = senDireito < threshold;
    // Detectar flanco de subida na direita (só conta quando passa de fora-da-linha para cima-da-linha)
    if (leDireita && !estadoAnteriorDireita) {
        contadorDireita++;
        Serial.print("Flanco detectado na direita. Contador: ");
        Serial.println(contadorDireita);

        if (contadorDireita >= MAX_DIREITA) {
            // anda um cadinho antes de parar
            motors.setSpeeds(20, 20);
            delay(500);

            motors.setSpeeds(0, 0);
            seguindoLinha = false;
            Serial.println("Limite de flags da direita atingido. Robô PAROU.");
        }
    }
    // Atualiza o estado anterior
    estadoAnteriorDireita = leDireita;
  }
}


// ------ CALIBRAÇÃO DOS SENSORES ------
void calibrarSensores() {
  // Calibração manual
  Serial.println("INICIANDO CALIBRACAO...");
  delay(500); 

  for (uint16_t i = 0; i < 100; i++) { 
    // motors.setSpeeds(30, 30); 
    // motors.setSpeeds(-30, -30); 
    qtr.calibrate(); 
    delay(25); 
  }

  motors.setSpeeds(0, 0); 
  Serial.println("CALIBRACAO FINALIZADA!");
}