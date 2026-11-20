#include <Servo.h>

Servo servo_gauche_haut;
Servo servo_gauche_bas;
Servo servo_droit_haut;
Servo servo_droit_bas;
Servo servo_theta;
Servo servo_phi;

void setup() 
{
  servo_theta.attach(2); 
  servo_phi.attach(A0);  
  servo_gauche_haut.attach(4);
  servo_gauche_bas.attach(5);
  servo_droit_bas.attach(6);
  servo_droit_haut.attach(7);
}

void loop() 
{
  servo_gauche_haut.write(100);
  servo_gauche_bas.write(95);
  servo_droit_haut.write(100);
  servo_droit_bas.write(95);
  servo_theta.write(90);
  servo_phi.write(90);
  
}

