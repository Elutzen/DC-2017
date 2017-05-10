/*************************************
 * code for the hunted robot
 * as often as possible, read the opponent position from the xbee
 * the UART4 RX pin is 31 (second from the bottom left)
 * the xbee needs 3.3v, gnd, and dout
 * 
 * and read the vive position sensor on pin 24
 * 
*************************************/
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define V1PIN 24 // the signal from the sensor
#define DEG_PER_US 0.0216 // equal to (180 deg) / (8333 us)
#define LIGHTHOUSEHEIGHT 6.0 // in feet

#define MOTORDIR1 38
#define MOTORSPD1 36
#define MOTORDIR2 37 
#define MOTORSPD2 35
#define XRAWLEN -1
#define YRAWLEN -1

// structure to store the sensor data
typedef struct {
  unsigned long changeTime[11];
  double horzAng;
  double vertAng;
  int useMe;
  int collected;
} viveSensor;

// variables for the sensor data and filter
volatile viveSensor V1;
int state = 0;
double xPos, yPos;
double xOld = 0, yOld = 0, xFilt = 0, yFilt = 0;

// variables for the xbee data
char msg[100];
int msg_index = 0;
float xOpponent, yOpponent;


void changeSpd(int newSpd) {
  analogWrite(MOTORSPD1, newSpd);
  analogWrite(MOTORSPD2, newSpd);
}

void turnRight() {
  changeSpd(0);
  delay(100);
  digitalWrite(MOTORDIR1, LOW);
  digitalWrite(MOTORDIR2, LOW);
  changeSpd(100); 
  delay(345);
  changeSpd(0);
  digitalWrite(MOTORDIR1, LOW);
  digitalWrite(MOTORDIR2, HIGH);
  delay(100);
}

void turnLeft() {
  changeSpd(0);
  delay(100);
  digitalWrite(MOTORDIR1, HIGH);
  digitalWrite(MOTORDIR2, HIGH);
  changeSpd(100);
  delay(345);
  changeSpd(0);
  digitalWrite(MOTORDIR1, LOW);
  digitalWrite(MOTORDIR2, HIGH);
  delay(100);
}

float distance(float myX, float myY, float yourX, float yourY) {
  return sqrt((myX-yourX)*(myX-yourX) + (myY-yourY)*(myY-yourY));
}

void runTrack() {
    delay(3000);
    turnRight();
    changeSpd(200);
    delay(1500);
    turnRight();
    changeSpd(200);
    delay(3000);
    turnRight();
    changeSpd(200);
    delay(1500);
    while(1) {
      changeSpd(0);
    }
}

void setup() {
  Serial.begin(9600); // to talk to the computer
  Serial4.begin(9600); // to listen to the xbee
  pinMode(13, OUTPUT); // to blink the led on pin 13
  pinMode(V1PIN, INPUT); // to read the sensor

  // initialize the sensor variables
  V1.horzAng = 0;
  V1.vertAng = 0;
  V1.useMe = 0;
  V1.collected = 0;
  // interrupt on any sensor change
  attachInterrupt(digitalPinToInterrupt(V1PIN), ISRV1, CHANGE);

    //PWM Pin Setup
  pinMode(MOTORDIR2, OUTPUT);
  pinMode(MOTORDIR1, OUTPUT);
  pinMode(MOTORSPD1, OUTPUT);
  pinMode(MOTORSPD2, OUTPUT);
  digitalWrite(MOTORDIR1, LOW);
  digitalWrite(MOTORDIR2, HIGH);
  changeSpd(0);
}

void loop() {
  // see if the xbee has sent any data
  if (Serial4.available() > 0) {
    msg[msg_index] = Serial4.read();
    // if you get a newline, there is data to read
    if (msg[msg_index] == '\n') {
      msg_index = 0;
      // data is in the format of two floats seperated by spaces
      sscanf(msg, "%f %f", &xOpponent, &yOpponent);

      Serial.print("op: ");
      Serial.print(xOpponent);
      Serial.print(" ");
      Serial.println(yOpponent);
    }
    else {
      // did not get a newline yet, just store for later
      msg_index++;
      if (msg_index == 100) {
        msg_index = 0;
      }
    }
  }

  // if the sensor data is new
  if (V1.useMe == 1) {
    V1.useMe = 0;

    // calculate the position and filter it
    xPos = tan((V1.vertAng - 90.0) * DEG_TO_RAD) * LIGHTHOUSEHEIGHT;
    yPos = tan((V1.horzAng - 90.0) * DEG_TO_RAD) * LIGHTHOUSEHEIGHT;
    xFilt = xOld * 0.5 + xPos * 0.5;
    yFilt = yOld * 0.5 + yPos * 0.5;
    xOld = xFilt;
    yOld = yFilt;
    Serial.print(xFilt);
    Serial.print(" ");
    Serial.println(yFilt);



    // blink the led so you can tell if you are getting sensor data
    digitalWrite(13, state);
    if (state == 1) {
      state = 0;
    }
    else {
      state = 1;
    }
  }
}

// the sensor interrupt
void ISRV1() {
  // get the time the interrupt occured
  unsigned long mic = micros();
  int i;

  // shift the time into the buffer
  for (i = 0; i < 10; i++) {
    V1.changeTime[i] = V1.changeTime[i + 1];
  }
  V1.changeTime[10] = mic;

  // if the buffer is full
  if (V1.collected < 11) {
    V1.collected++;
  }
  else {
    // if the times match the waveform pattern
    if ((V1.changeTime[1] - V1.changeTime[0] > 7000) && (V1.changeTime[3] - V1.changeTime[2] > 7000) && (V1.changeTime[6] - V1.changeTime[5] < 50) && (V1.changeTime[10] - V1.changeTime[9] < 50)) {
      V1.horzAng = (V1.changeTime[5] - V1.changeTime[4]) * DEG_PER_US;
      V1.vertAng = (V1.changeTime[9] - V1.changeTime[8]) * DEG_PER_US;
      V1.useMe = 1;
    }
  }
}



