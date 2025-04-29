#ifndef MOTOR_H // Garante que o código será incluído apenas uma vez
#define MOTOR_H

class motor {
    
private:    
    int pinIN1;            // Pino de controle 1 
    int pinIN2;            // Pino de controle 2
    int canal1;            // Canal de PWM 1
    int canal2;            // Canal de PWM 2
    int frequencia;        // Frequência do PWM
    int resolucao;         // Resolução do PWM

    void config();         // Método privado para configurar os canais PWM dos motores

public:
    motor(int _pinIN1, int _pinIN2, int _canal1, int _canal2, int _frequencia, int _resolucao); // Construtor
    void controlarPWM(int velocidade); // Controla a velocidade do motor
};

#endif
