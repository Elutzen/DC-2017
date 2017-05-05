#include <avr/io.h>
//pin 38(digital) with pin 36 (pwm)
int motorDir1 = 38;
int motorAn1 = 36;
//pin 37(digital) with pin 35 (pwm)
int motorDir2 = 37;
int motorAn2 = 35;

void setup() {
  //PWM Pin Setup
  pinMode(motorDir2, OUTPUT);
  pinMode(motorDir1, OUTPUT);
  pinMode(motorAn1, OUTPUT);
  pinMode(motorAn2, OUTPUT);
  digitalWrite(motorDir1, HIGH);
  digitalWrite(motorDir2, LOW);
}

void loop() {
  analogWrite(motorAn1, 120);
  analogWrite(motorAn2, 120);
  delay(5000);
  digitalWrite(motorDir2, HIGH);
  digitalWrite(motorDir1, LOW);

  delay(5000);
  digitalWrite(motorDir1, HIGH);
  digitalWrite(motorDir2, LOW);
  delay(5000);
  digitalWrite(motorDir1, LOW);
  delay(5000);
  while (1) {
    analogWrite(motorAn1, 0);
    analogWrite(motorAn2, 0);
  }
}



