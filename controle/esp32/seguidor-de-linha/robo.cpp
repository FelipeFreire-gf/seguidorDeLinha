#include "robo.h"

robo::robo(motor *_m1, motor *_m2) : m1(_m1), m2(_m2) {}

void robo::irParaFrente(int velocidade1, int velocidade2)
{
    // ajusta a velocidade de cada motor para mover o robô para frente
    m1->controlarPWM(velocidade1);
    m2->controlarPWM(velocidade2);
}

void robo::parar()
{
    // para ambos os motores
    m1->controlarPWM(0); // motor 1 parado
    m2->controlarPWM(0); // motor 2 parado
}

void robo::irParaTras(int velocidade1, int velocidade2)
{
    // ajusta a direção para mover o robô para trás
    m1->controlarPWM(-velocidade1);
    m2->controlarPWM(-velocidade2);
}

void robo::virarEsquerda(int velocidade)
{
    // motor 1 vai para trás e motor 2 vai para frente, fazendo o robõ virar para a esquerda
    m1->controlarPWM(-velocidade);
    m1->controlarPWM(velocidade);
}

void robo::virarDireita(int velocidade)
{
    // motor 1 vai para frente e motor 2 vai para trás, fazendo o robõ virar para a direita
    m1->controlarPWM(velocidade);
    m1->controlarPWM(-velocidade);
}