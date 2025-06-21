#include <QTRSensors.h>
#include <OrangutanLEDs.h>
#include <OrangutanMotors.h>

OrangutanLEDs leds;
OrangutanMotors motors;

#define SEN_D A6                        // ADC6
#define BRUSHED 8                       // PB0
#define LOWH 10                         // PB2 (força sinal PWM low na ponte h)
#define MAX_DIREITA 26                  // Quantidade máxima de vezes que o sensor da direita lê linhas no circuito
#define LED_SENSOR 2

QTRSensors qtr;

const uint8_t sensorCount = 6;        // ALTERAR CASO ESTEJA USANDO SOMENTE 6 SENSORES
//const uint8_t sensorCount = 8;
uint16_t sensorValues[sensorCount];

// ------------ Sensores Laterais --------------
// sensores vão de 0(branco) a 1000(preto)
int contadorDireita = 0;                // Conta quantas vezes leu a linha da direita
const int threshold = 910;              // Valor intermediario em que o sensor considera como preto ou branco

bool seguindoLinha = false;
bool modoLento = false;
bool usandoLinhaPreta = false;
bool estadoAnteriorDireita = false;

int velocidadeLenta = 60;               // Velocidade base para o modo lento
int centro = 2500;                      // Posição central corrigida dos sensores (0-5000)


// ------------ CONSTANTES PID ------------
/*
  TODOS OS TESTES FORAM FEITOS COM V75
  TESTE 1 = SEGUIU LINHA
  Kp = 0.04
  Ki = 0.0005
  Kd = 0.5
  TESTE 2 = ROBÔ SEGUIU LINHA, MAS NÃO FOI BEM
  Kp = 0.04
  Ki = 0.0
  Kd = 0.0
  TESTE 3 = FRACASSO
  Kp = 0.1
  Ki = 0.0
  Kd = 0.0
  TESTE 4 = ROBÔ SEGUIU LINHA, MAS NÃO FOI BEM
  Kp = 0.06
  Ki = 0.0
  Kd = 0.0
  TESTE 5 = ROBÔ SEGUIU LINHA, FOI MELHOR QUE O TESTE 1
  Kp = 0.04
  Ki = 0.0
  Kd = 0.1
  TESTE 6 = ROBÔ SEGUIU LINHA, FOI MELHOR QUE O TESTE 5
  Kp = 0.04
  Ki = 0.0
  Kd = 0.2
  TESTE 7 = ROBÔ SEGUIU LINHA, FOI MELHOR QUE O TESTE 6
  Kp = 0.04
  Ki = 0.0
  Kd = 0.3
  TESTE 8 = ROBÔ SEGUIU LINHA, FOI MELHOR QUE O TESTE 7
  Kp = 0.04
  Ki = 0.0
  Kd = 0.4
  TESTE 9 = ROBÔ SEGUIU LINHA, FOI MUITO MELHOR QUE O TESTE 8
  Kp = 0.04
  Ki = 0.0
  Kd = 0.5
  TESTE 10 = ROBÔ SEGUIU LINHA, FOI MELHOR QUE O TESTE 9
  Kp = 0.04
  Ki = 0.0
  Kd = 0.7
  TESTE 11 = ROBÔ SEGUIU LINHA, AINDA NÃO SEI JULGAR SE FOI MELHOR QUE O 10
  Kp = 0.04
  Ki = 0.0
  Kd = 0.8
  TESTE 12 = ROBÔ SEGUIU LINHA, TIVE A IMPRESSÃO QUE O TESTE 11 FOI MELHOR
  Kp = 0.04
  Ki = 0.0
  Kd = 1.2
  TESTE 13 = ROBÔ SEGUIU LINHA MAL, CORREÇÃO MUITO GROSSEIRA
  Kp = 0.04
  Ki = 0.0
  Kd = 1.7
  TESTE 15 = ROBÔ SEGUIU LINHA MAL, APENAS ME CONFIRMOU QUE O KD ESTÁ ALTO
  Kp = 0.04
  Ki = 0.0
  Kd = 1.5
  TESTE 15 = ROBÔ SEGUIU LINHA, DERRAPANDO UM POUCO
  Kp = 0.04
  Ki = 0.0
  Kd = 1.36
  TESTE 16 = ROBÔ SEGUIU LINHA, ME FEZ PERCEBER QUE O TESTE 12 FOI MELHOR
  Kp = 0.04
  Ki = 0.0
  Kd = 1.3
  TESTE 17 = ROBÔ SEGUIU LINHA, MUITO MELHOR QUE O TESTE 16
  Kp = 0.04
  Ki = 0.0
  Kd = 1.0
  TESTE 18 = ROBÔ SEGUIU LINHA, PIOR QUE O TESTE 17
  Kp = 0.04
  Ki = 0.0
  Kd = 0.9
  TESTE 19 = ROBÔ SEGUIU LINHA, DE LONGE O MELHOR TESTE
  Kp = 0.04
  Ki = 0.0
  Kd = 1.02
*/
double Kp = 0.04;
double Ki = 0.0;
double Kd = 1.02;

int erroAnterior = 0;
long erroIntegral = 0; 

void setup() {
  pinMode(LED_SENSOR, OUTPUT);
  digitalWrite(LED_SENSOR, HIGH); 

  pinMode(LOWH, OUTPUT);

  pinMode(SEN_D, INPUT);
  pinMode(BRUSHED, OUTPUT);

  qtr.setTypeRC(); 
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5}, sensorCount);          // Sensores de 1 a 6 (caso 8 sensores estiverem falhando)
  //qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5, 7, 4}, sensorCount);    // Sensores de 1 a 8 (prototipo 3)

  qtr.setEmitterPin(LED_SENSOR);

  motors.setSpeeds(0, 0); 
  Serial.begin(9600); 
  delay(100); 
}

void loop() {
  /*
      LISTA DE COMANDOS:
      0 = LEITURA DOS SENSORES
      1 = ATIVAÇÃO DOS MOTORES
      2 = DESLIGAMENTO DOS MOTORES
      3 = MODO: SEGUIDOR DE LINHA NORMAL
      4 = CALIBRAÇÃO DOS SENSORES FRONTAIS
      5 = MODO: SEGUIDOR DE LINHA LENTO
      6 = MOSTRA PARÂMETROS ATUAIS (VELOCIDADE, CONSTANTES PID E QUAL COR DE LINHA SEGUE)
      7 = ATIVAÇÃO DOS MOTORES BRUSHED
      p = AJUSTA CONSTANTE P
      i = AJUSTA CONSTANTE I
      d = AJUSTA CONSTANTE D
      v = AJUSTA VELOCIDADE LENTA (DEVE SER SEGUIDO DE ALGUM VALOR COMO "v60")
      c = AJUSTA CENTRO (DEVE SER SEGUIDO DE ALGUM VALOR COMO "c3500")
      b = MODO: SEGUIR LINHA PRETA
      w = MODO: SEGUIR LINHA BRANCA
      r = MOSTRA VALOR DO SENSOR LATERAL DIREITO
  */
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando == "1") {
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
      qtr.read(sensorValues); 
      Serial.println("Leitura dos sensores:");
      for (uint8_t i = 0; i < sensorCount; i++) {
        Serial.print("Sensor ");
        Serial.print(i+1);
        Serial.print(": ");
        Serial.println(sensorValues[i]);
      }
      // Calcula a posição da linha (0 a 7000)
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
    else if (comando == "7"){
      digitalWrite(BRUSHED, HIGH);
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
    else if (comando == "r"){
      int direita = analogRead(SEN_D);
      Serial.println(direita);
    }
  }

  // --- MODO PID: Seguir Linha ---
  if (seguindoLinha) {
    digitalWrite(BRUSHED, HIGH);
    digitalWrite(LOWH, LOW);
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
    int velEsq = velocidadeBase - correcao;
    int velDir = velocidadeBase + correcao;
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
          motors.setSpeeds(40, 40);
          delay(100);

          motors.setSpeeds(0, 0);
          seguindoLinha = false;
          delay(100000);
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
