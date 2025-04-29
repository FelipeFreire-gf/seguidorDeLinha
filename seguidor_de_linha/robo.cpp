#include "robo.h"

Robo::Robo(Motor* _m1, Motor* _m2): m1(_m1), m2(_m2)
{}
void Robo::moveFoward(int velocidade1, int velocidade2){
    m1->setPWM(velocidade1);
    m2->setPWM(velocidade2);
}
void Robo::Stop(){
    m1->setPWM(0);
    m2->setPWM(0);
}