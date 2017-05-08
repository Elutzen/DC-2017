#include <string>
#include <avr/io.h>
#include <math.h>
#include <stdio.h>
//pin 38(digital) with pin 36 (pwm)
#define MOTORDIR1 38
#define MOTORSPD1 36
#define MOTORDIR2 37 
#define MOTORSPD2 35
#define XRAWLEN -1
#define YRAWLEN -1

void parseData(char *raw, int *x, int *y) {
  
  String allData(raw); 
  float x;
  float y;

  String strX = allData.substring(0, allData.indexOf(' '));
  String strY = allData.substring(allData.indexOf(' '));
  x = strX.toFloat();
  y = strY.toFloat();
  
}

int distance(int myX, int myY, int yourX, int yourY) {
  return sqrt((myX-yourX)*(myX-yourX) + (myY-yourY)*(myY- yourY));
}

void changeSpd(int newSpd) {
  analogWrite(MOTORSPD1, newSpd);
  analogWrite(MOTORSPD2, newSpd);
}

void turnRight() {
  digitalWrite(MOTORDIR1, LOW);
  digitalWrite(MOTORDIR2, LOW);
  changeSpd(255); 
  delay(135);
  changeSpd(0);
}

void turnLeft() {
  digitalWrite(MOTORDIR1, HIGH);
  digitalWrite(MOTORDIR2, HIGH);
  changeSpd(255);
  delay(135);
  changeSpd(0);
}

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


//  int *x;
//  int *y;
//  parseData(some pin, &x, &y);

  for(int i = 0; i < 4; i++) {
    if (i < 2) {
      turnRight();
      delay(5000);
    }
    else {
      turnLeft();
      delay(5000);
    }
  }
  while (1) {
    
  }

}



