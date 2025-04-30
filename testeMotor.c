#include <OrangutanLEDs.h>
#include <OrangutanMotors.h>
OrangutanLEDs leds;
OrangutanMotors motors;
void setup()
{
  motors.setSpeeds(200, 200);  
  leds.red(HIGH);
}
void loop()
{}