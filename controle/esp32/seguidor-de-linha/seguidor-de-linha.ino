#include <Arduino.h>
#include "motor.h"
#include "robo.h"

// definir os pinos dos sensores 
const int sensorEsquerdoPin = 2;
const int sensorDireitoPin = 3;

//definir os pinos dos motores 
const int motorEsquerdoA = 9;
const int motorEsquerdoB = 10;
const int motorDireitoA = 6;
const int motorDireitoB = 5;

// constantes do PID
const float Kp = 1.0;       // ganho proporcional
const float Ki = 0.1;       // ganho integral 
const float Kd = 0.5;       // ganho derivativo

// variáveis para PID
float erro = 0;             // erro atual
float erroAcumulado = 0;    // somo dos erros anteriores
float erroAnterior = 0;    // erro do ciclo anterior 

// objetos para os motores e robõ
motor motor1(motorEsquerdoA, motorEsquerdoB, 1, 2, 1000, 8);  //motor 1
motor motor2(motorDireitoA, motorDireitoB, 3, 4, 1000, 8);    //motor 2
robo robo(&motor1, &motor2);        // instância do robô com os motores 

void setup(){
    // configurações iniciais para os sensores
    pinMode(sensorEsquerdoPin, INPUT);      // configure o sensor esquerdo como entrada
    pinMode(sensorDireitoPin, INPUT);       // configure o sensor direitoo como entrada

    // inicializa a comunicação serial para depuração
    Serial.begin(9600);
}

void loop(){
    // leitura dos sensores de linha 
    int sensorEsquerdo = digitalRead(sensorEsquerdoPin);
    int sensorDireito = digitalRead(sensorDireitoPin);

    // cálculo do erro (distância entre a posição atual e a desejada)
    erro = sensorEsquerdo - sensorDireito;

    // cálculo do componente proporcional (P)
    float P = Kp * erro;

    // cálculo do componente integral (I)
    erroAcumulado += erro;
    float I = Ki * erroAcumulado;
    
    // cálculo do componente derivativo (D)
    float D = Kd * (erro - erroAnterior);

    // cálculo do sinal de contrle total
    float controle = P + I + D;

    // aplica o controle ao movimento do robô
    aplicarControlePID(controle);

    // atualiza o erro anterior para o próximo ciclo
    erroAnterior = erro;
    
    // exibe os valores no monitor serial para monitoramento
    Serial.print("Erro: ");
    Serial.print(erro);
    Serial.print("   Controle PID ");
    Serial.println(controle);

    delay(100);  // delay para a próxima leitura
}

// função para aplicar o controle PID nos motores
void aplicarControlePID(float controle){
    controle = constrain(controle, -255, 255); //limita o controle entre -255 e 255

    if (controle > 0){
        // o robô vira para dirita 
        robo.virarDireita(255 - controle);  // motor esquerdo gira mais devagar
    } else if (controle < 0){
        // o robô vira para esquerda 
        robo.virarEsquerda(255 + controle); // motor direito gira mais devagar 
    } else {
        // o robô vai para frente
        robo.irParaFrente(255, 255);        // ambos os motores vão a maxima velocidade
    }
}