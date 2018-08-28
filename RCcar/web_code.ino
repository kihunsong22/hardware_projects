#include <AFMotor.h>

AF_DCMotor motor1(3);
AF_DCMotor motor2(4);

void setup() {
    motor1.setSpeed(150);   // MAX 255
    motor2.setSpeed(150);
    motor1.run(RELEASE);
    motor2.run(RELEASE);
}

void loop() {
    motor1.run(FORWARD);
    motor2.run(FORWARD);
    delay(150);
    motor1.run(RELEASE);
    motor2.run(RELEASE);
    delay(1000);
    
    motor1.run(BACKWARD);
    motor2.run(BACKWARD);
    delay(150);
    motor1.run(RELEASE);
    motor2.run(RELEASE);
    delay(1000);
}