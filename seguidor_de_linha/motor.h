#ifndef MOTOR_H
#define MOTOR_H




class Motor{
private:                    // somente acessados por meio de classes
    int pinIN1;
    int pinIN2;
    int canal1;
    int canal2;
    int frequencia;
    int resolucao;

    void config();                  // pega os atributos e configura em cada variavel da classe motor
public:
    Motor(int _pinIN1, int _pinIN2, int _canal1, int _canal2, int _frquencia, int _resolucao);   // "_" por convenção, "canal" é um seletor

    void setPWM(int velocidade);
};

#endif