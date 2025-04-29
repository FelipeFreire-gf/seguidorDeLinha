#ifndef ROBO_H
#define ROBO_H
#include "motor.h"

class robo {
    private:
        motor* m1; // motor 1
        motor* m2; // motor 2

    public:

        robo(motor* _m1, motor* _m2); // construtor 
      
        // move o robô para frente 
        void irParaFrente(int velocidade1, int velocidade2);
        
        // parar o robô
        void parar(); 

        // move o robô para trás
        void irParaTras(int velocidade1, int velocidade2);

        // move o robô para a esquerda
        void virarEsquerda(int velocidade);
        
        // move o robô para a direita
        void virarDireita(int velocidade);
    
};

#endif