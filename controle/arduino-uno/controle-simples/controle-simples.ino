#include <Arduino.h>

// Definir Pinos dos sensores
const int sensorEsquerdoPin = 2;
const int sensorDireitoPin = 3;

// Definir pinos dos motores
const int motorEsquerdoA = 9;
const int motorEsquerdoB = 10;
const int motorDireitoA = 6;
const int motorDireitoB = 5;

// Constantes PID
const float Kp = 1.0;  // Ganho Proporcional
const float Ki = 0.1;  // Ganho Integral
const float Kd = 0.5;  // Ganho Derivativo

// Variaveis para  o PID
float erro = 0;           // erro atual
float erroAcumulado = 0;  //soma dos erros anteriores
float erroAnterior = 0;   //Erro do ciclo anterior


void setup() {
  // Função setup() é executada uma única vez quando o Arduino é ligado ou reiniciado.
  // Aqui, configuramos os pinos de entrada para os sensores e os pinos de saída para os motores.
  // Também inicializamos a comunicação serial para monitoramento e depuração.
  // É importante que tudo o que precisa ser configurado antes do loop comece seja feito no setup().

  // inicializando os pinos dos sensores como entradas
  pinMode(sensorEsquerdoPin, INPUT);
  pinMode(sensorDireitoPin, INPUT);

  // inicializando os pinos dos motores como saídas
  pinMode(motorEsquerdoA, OUTPUT);
  pinMode(motorEsquerdoB, OUTPUT);
  pinMode(motorDireitoA, OUTPUT);
  pinMode(motorDireitoB, OUTPUT);

  // Comunicação Serial
  Serial.begin(9600);
}

void loop() {
  // A função é executada repetidamente pelo Arduino. Ela é a função principal do código,
  // onde a lógica de controle deve ser repetida continuamente,
  // como ler os sensores e ajustar os motores.

  // LOW significa que o sensor está detectando a linha (geralmente preto ou escuro, dependendo do tipo de sensor)
  // HIGH significa que o sensor está fora da linha (geralmente detectando o fundo, que é claro, como branco)

  // Lendo o estado dos sensores de linha
  int sensorEsquerdo = digitalRead(sensorEsquerdoPin);
  int sensorDireito = digitalRead(sensorDireitoPin);

  //Cálculo do erro (distância entre a posição atual e a desejada)
  erro = sensorEsquerdo - sensorDireito;  // Pode ser ajustado

  //Cálculo do componente proporcional (P)
  float P = Kp * erro;

  //Cálculo do componente integral (I)
  erroAcumulado += erro;
  float I = Ki * erroAcumulado;

  //Cálculo do componente derivativo (D)
  float D = Kd * (erro - erroAnterior);

  //Cálculo do sinal de controle total
  float controle = P + I + D;

  //Aplica o controle aos motores
  aplicarControlePID(controle);

  //Armazenando o erro atual para o próximo ciclo
  erroAnterior = erro;


  // Exibindo o estado dos sensores no monitor serial
  Serial.print("Erro: ");
  Serial.print(erro);
  Serial.print("   controle PID ");
  Serial.println(controle);

  delay(100);  // Aguarda um pouco antes de repetir a leitura dos sensores
}

// Função parra aplicar o controle do PID nos motores com PWd 
void aplicarControlePID(float controle) {

  // Ajusta a velocidade dos motores com base no sinal do controle PID
  controle = constrain(controle, -255, 255); // Limita o controle entre -255 e 255

  int velocidadeEsquerda = 255;
  int velocidadeDireita = 255;

  if (controle > 0) {
    //O robô vira para a direita
    analogWrite(motorEsquerdoA, 255 - controle); // Motor esquerdo gira para a esquerda (menor velocidade)
    digitalWrite(motorEsquerdoB, LOW);
    analogWrite(motorDireitoA, 255); // Motor direito gira para frente com maxima velocidade 
    digitalWrite(motorDireitoB, LOW);
  } else if (controle < 0) {
    //O robô vira para a esquerda
    analogWrite(motorEsquerdoA, 255); // Motor esquerdo gira para frente com máxima velocidade
    digitalWrite(motorEsquerdoB, LOW);
    analogWrite(motorDireitoA, 255 + controle); // Motor direito gira para a direita com velocidade ajustada
    digitalWrite(motorDireitoB, LOW);
  } else {
    //O robô vai para frente
    analogWrite(motorEsquerdoA, 255); // Motor esquerdo gira para frente com máxima velocidade
    digitalWrite(motorEsquerdoB, LOW);
    analogWrite(motorDireitoA, 255); // Motor direito gira para frente com máxima velocidade
    digitalWrite(motorDireitoB, LOW);
  }
}