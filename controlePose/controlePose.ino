#include <QTRSensors.h>
#include <OrangutanMotors.h>

//--------- Contadores de pulsos de cada encoder -----------  
volatile long pulsosAtualRodaDireita = 0;  // Contador de pulsos roda direita
volatile long pulsosAtualRodaEsquerda = 0; // Contador de pulsos roda esquerda

//--------- Parâmetros PID -----------  
float Kp_roda = 0.25;
float Ki_roda = 0.5;
float Kd_roda = 0.0625;

float integralRodaDireita = 0;
float integralRodaEsquerda = 0;
float erroAnteriorRodaDireita = 0;
float erroAnteriorRodaEsquerda = 0;
float intervaloTempo = 0.02; // intervalo 20ms

//--------- Definindo os pinos para os encoders -----------  
#define PINO_ENCODER_DIREITA PB0  // roda direita
#define PINO_ENCODER_ESQUERDA PB1 // roda esquerda

OrangutanMotors motores; 

//--------- Configuração dos encoders para interrupções -----------  
void configurarEncoderInterrupcao() {
    DDRB &= ~(1 << DDB0);  // PB0 como entrada
    DDRB &= ~(1 << DDB1);  // PB1 como entrada
    PORTB |= (1 << PORTB0); // pull-up PB0
    PORTB |= (1 << PORTB1); // pull-up PB1

    PCICR |= (1 << PCIE0); // habilita interrupção grupo B
    PCMSK0 |= (1 << PCINT0) | (1 << PCINT1); // habilita PCINT0 e PCINT1

    sei(); // habilita interrupções globais
}

//--------- ISR para interrupção de mudança de pino -----------  
ISR(PCINT0_vect) {
    if (PINB & (1 << PINB0)) pulsosAtualRodaDireita++;
    if (PINB & (1 << PINB1)) pulsosAtualRodaEsquerda++;
}

//--------- Função de controle PID para uma roda -----------  
int controlePID(long pulsosAtual, long setpoint, float &integral, float &erroAnterior) {
    float erro = setpoint - pulsosAtual;
    integral += erro * intervaloTempo;
    float derivativo = (erro - erroAnterior) / intervaloTempo;
    float saida = Kp_roda * erro + Ki_roda * integral + Kd_roda * derivativo;
    erroAnterior = erro;
    if (saida > 255) saida = 255;
    else if (saida < -255) saida = -255;
    return (int)saida;
}

//--------- Chamada no loop para controle das duas rodas -----------  
void controlePosicional() {
    int pwmDireita = controlePID(pulsosAtualRodaDireita, 1000, integralRodaDireita, erroAnteriorRodaDireita);
    int pwmEsquerda = controlePID(pulsosAtualRodaEsquerda, 1000, integralRodaEsquerda, erroAnteriorRodaEsquerda);

    // Como setSpeeds espera esquerda, direita
    motores.setSpeeds(pwmEsquerda, pwmDireita);
}



//--------- Função setup -----------  
void setup() {
    configurarEncoderInterrupcao();
    
    // Resetar variáveis e Inicializa os motores
    pulsosAtualRodaDireita = 0;
    pulsosAtualRodaEsquerda = 0;
    integralRodaDireita = 0;
    integralRodaEsquerda = 0;
    erroAnteriorRodaDireita = 0;
    erroAnteriorRodaEsquerda = 0;
}

//--------- Função loop -----------  
void loop() {
    controlePosicional();
    delay(20); // Mantém o intervalo constante para o PID
}
