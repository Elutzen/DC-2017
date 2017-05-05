#include <avr/io.h>
//pin 38(digital) with pin 36 (pwm)
#define MOTORDIR1 38
#define MOTORSPD1 36
#define MOTORDIR2 37 
#define MOTORSPD 35

//int MOTORDIR1 = 38;
//int MOTORSPD1 = 36;
////pin 37(digital) with pin 35 (pwm)
//int MOTORDIR2 = 37;
//int MOTORSPD2 = 35;

void setup() {
  //PWM Pin Setup
  pinMode(MOTORDIR2, OUTPUT);
  pinMode(MOTORDIR1, OUTPUT);
  pinMode(MOTORSPD1, OUTPUT);
  pinMode(MOTORSPD2, OUTPUT);
  digitalWrite(MOTORDIR1, HIGH);
  digitalWrite(MOTORDIR2, LOW);
}

void loop() {
  analogWrite(MOTORSPD1, 120);
  analogWrite(MOTORSPD2, 120);
  delay(2000);
  digitalWrite(MOTORDIR2, HIGH);
  digitalWrite(MOTORDIR1, LOW);

  delay(2000);
  digitalWrite(MOTORDIR1, HIGH);
  digitalWrite(MOTORDIR2, LOW);
  delay(2000);
  digitalWrite(MOTORDIR1, LOW);
  delay(1000);
  while (1) {
    analogWrite(MOTORSPD1, 0);
    analogWrite(MOTORSPD2, 0);
  }
}



