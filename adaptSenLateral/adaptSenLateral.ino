#include <QTRSensors.h>
#include <OrangutanLEDs.h>
#include <OrangutanMotors.h>

OrangutanLEDs leds;
OrangutanMotors motors;

#define SEN_E A6     // ADC6
#define SEN_D A0     // PC0
#define LED_SENSOR 2

QTRSensors qtr;

const uint8_t sensorCount = 6;
uint16_t sensorValues[sensorCount];

// ------------ Sensores Laterais --------------
int contadorDireita = 0;                // Conta quantas vezes leu a linha da direita
const int threshold = 500;              // Valor intermeidario em que o sensor considera como preto ou branco

int dadoBluetooth = 0;
bool seguindoLinha = false;
bool modoLento = false;
bool usandoLinhaPreta = false; // true: segue linha preta; false: segue linha branca

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
  
  pinMode(SEN_E, INPUT);
  pinMode(SEN_D, INPUT);

  // Define sensores QTR com os pinos conectados
  qtr.setTypeRC();
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

    // ----------- COMANDOS DE CONTROLE MANUAL -----------
    if (comando == "1") {
      // Liga os motores (velocidade 200) — modo manual
      motors.setSpeeds(200, 200);
      seguindoLinha = false;
      Serial.println("Motores LIGADOS");
    }
    else if (comando == "2") {
      // Desliga os motores
      motors.setSpeeds(0, 0);
      seguindoLinha = false;
      Serial.println("Motores PARADOS");
    }
    else if (comando == "0") {
      // Lê e imprime os valores atuais dos sensores
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

    // ----------- MODOS DE SEGUIDOR DE LINHA -----------
    else if (comando == "3") {
      // Ativa o modo de seguidor de linha NORMAL
      seguindoLinha = true;
      modoLento = false;
      erroAnterior = 0;
      erroIntegral = 0;
      Serial.println("MODO: SEGUIDOR DE LINHA NORMAL ATIVADO");
    }
    else if (comando == "5") {
      // Ativa o modo de seguidor de linha LENTO
      seguindoLinha = true;
      modoLento = true;
      erroAnterior = 0;
      erroIntegral = 0;
      Serial.println("MODO: SEGUIDOR DE LINHA LENTO ATIVADO");
    }

    // ----------- CALIBRAÇÃO DOS SENSORES -----------
    else if (comando == "4") {
      calibrarSensores(); // Executa a calibração dos sensores
    }

    // ----------- AJUSTES E INFORMAÇÕES -----------
    else if (comando == "6") {
      // Mostra os parâmetros atuais
      Serial.print("Kp: "); Serial.println(Kp);
      Serial.print("Ki: "); Serial.println(Ki);
      Serial.print("Kd: "); Serial.println(Kd);
      Serial.print("Velocidade: "); Serial.println(velocidadeLenta);
      Serial.print("Modo: Seguindo linha ");
      Serial.println(usandoLinhaPreta ? "PRETA" : "BRANCA");
    }
    else if (comando.startsWith("p")) {
      // Altera o valor de Kp (ex: p0.08)
      Kp = comando.substring(1).toDouble();
      Serial.print("Novo Kp: "); Serial.println(Kp);
    }
    else if (comando.startsWith("i")) {
      // Altera o valor de Ki (ex: i0.001)
      Ki = comando.substring(1).toDouble();
      Serial.print("Novo Ki: "); Serial.println(Ki);
    }
    else if (comando.startsWith("d")) {
      // Altera o valor de Kd (ex: d0.4)
      Kd = comando.substring(1).toDouble();
      Serial.print("Novo Kd: "); Serial.println(Kd);
    }
    else if (comando.startsWith("v")) {
      // Altera a velocidade lenta (ex: v100)
      velocidadeLenta = comando.substring(1).toInt();
      Serial.print("Nova velocidade lenta: "); Serial.println(velocidadeLenta);
    }

    // ----------- DEFINIR TIPO DE LINHA -----------
    else if (comando == "b") {
      // Linha preta no fundo branco
      usandoLinhaPreta = true;
      Serial.println("Modo: seguir linha PRETA");
    }
    else if (comando == "w") {
      // Linha branca no fundo preto
      usandoLinhaPreta = false;
      Serial.println("Modo: seguir linha BRANCA");
    }
  }

  // ---------- LÓGICA DO SEGUIDOR DE LINHA ----------
  if (seguindoLinha) {
    uint16_t pos = usandoLinhaPreta ? qtr.readLineBlack(sensorValues)
                                     : qtr.readLineWhite(sensorValues);
    int erro = pos - 3500; // Posição ideal (meio) é 3500
    erroIntegral += erro;
    int derivada = erro - erroAnterior;

    int correcao = (int)(Kp * erro + Ki * erroIntegral + Kd * derivada);
    erroAnterior = erro;

    int velocidadeBase = modoLento ? velocidadeLenta : 150;

    int velEsq = velocidadeBase + correcao;
    int velDir = velocidadeBase - correcao;

    velEsq = constrain(velEsq, 0, 255);
    velDir = constrain(velDir, 0, 255);

    motors.setSpeeds(velEsq, velDir); // Aplica correção PID

    // Lógica dos sensores laterais
    int senDireito = analogRead(SEN_D);
    int senEsquerdo = analogRead(SEN_E);
    bool leDireita = senDireito > threshold;
    bool leEsquerda = senEsquerda > threshold;
    if (leDireita && !leEsquerda){
      if (contadorDireita == 0){
        Serial.println("Percurso iniciado");
        contadorDireita = 1;
      }
      else if(contadorDireita == 1){
        Serial.println("Percurso finalizado");
        motors.setSpeeds(0,0);
        contadorDireita = 0;
        seguindoLinha = false;
      }
    }
  }
}

// ----------- FUNÇÃO DE CALIBRAÇÃO DOS SENSORES -----------
void calibrarSensores() {
  Serial.println("INICIANDO CALIBRACAO...");
  delay(500);

  for (uint16_t i = 0; i < 50; i++) {
    motors.setSpeeds(80, 80); // Movimento lento durante calibração
    qtr.calibrate();
    delay(25);
  }

  motors.setSpeeds(0, 0);
  Serial.println("CALIBRACAO FINALIZADA!");

  // Imprime valores calibrados
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
