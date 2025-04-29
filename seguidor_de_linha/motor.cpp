#include <Arduino.h>
#include "motor.h"

Motor::Motor(int _pinIN1, int _pinIN2, int _canal1, int _canal2, int _frequencia, int _resolucao)
    : pinIN1(_pinIN1), pinIN2(_pinIN2), canal1(_canal1), canal2(_canal2), frequencia(_frequencia), resolucao(_resolucao)
{
    config();
}

void Motor::config() {
    // Configurar os canais de PWM
    ledcSetup(canal1, frequencia, resolucao);
    ledcSetup(canal2, frequencia, resolucao);

    // Associar os pinos aos canais
    ledcAttachPin(pinIN1, canal1);
    ledcAttachPin(pinIN2, canal2);
}

void Motor::setPWM(int velocidade) {
    if (velocidade > 0) {
        ledcWrite(canal1, velocidade);  // Ativa motor para frente
        ledcWrite(canal2, 0);           // Garante que o outro sentido está desligado
    }           
    else if (velocidade < 0) {
        ledcWrite(canal1, 0);           // Garante que o motor para frente está desligado
        ledcWrite(canal2, -velocidade); // Ativa motor para trás
    } else {
        ledcWrite(canal1, 0);
        ledcWrite(canal2, 0);
    }
}