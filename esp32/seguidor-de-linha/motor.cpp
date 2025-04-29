#include "motor.h"
#include <Arduino.h>

// Definindo os canais PWM
int canal1 = 1;  // Canal PWM 1
int canal2 = 2;  // Canal PWM 2

motor::motor(int _pinIN1, int _pinIN2, int _canal1, int _canal2, int _frequencia, int _resolucao) {
    pinIN1 = _pinIN1;
    pinIN2 = _pinIN2;
    canal1 = _canal1;
    canal2 = _canal2;
    frequencia = _frequencia;
    resolucao = _resolucao;

    pinMode(pinIN1, OUTPUT);
    pinMode(pinIN2, OUTPUT);

    // Configura os canais PWM
    ledcSetup(canal1, frequencia, resolucao);
    ledcSetup(canal2, frequencia, resolucao);

    // Associa os pinos aos canais PWM
    ledcAttachPin(pinIN1, canal1);
    ledcAttachPin(pinIN2, canal2);
}

void motor::controlarPWM(int velocidade) {
    if (velocidade > 0) {
        ledcWrite(canal1, velocidade);  // Motor vai para frente
        ledcWrite(canal2, 0);           // Motor para trás desativado
    } else if (velocidade < 0) {
        ledcWrite(canal1, 0);           // Motor vai para trás
        ledcWrite(canal2, -velocidade); // Motor para frente desativado
    } else {
        ledcWrite(canal1, 0);  // Para o motor
        ledcWrite(canal2, 0);  // Para o motor
    }
}
