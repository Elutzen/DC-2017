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

#define V1PIN 25 // the signal from the sensor for back
#define V2PIN 24 //the signal from the sensor for front
#define DEG_PER_US 0.0216 // equal to (180 deg) / (8333 us)
#define LIGHTHOUSEHEIGHT 7.5 // in feet

#define MOTORDIR1 38
#define MOTORSPD1 36
#define MOTORDIR2 37 
#define MOTORSPD2 35
#define XRAWLEN -1
#define YRAWLEN -1


//bump sensor pin modes
#define LEFTBUTTON 0
#define MIDDLEBUTTON 1
#define RIGHTBUTTON 2

//margin of error between vive sensors
#define TURNTHRESH 0.15
#define FACINGTHRESH 0.25
#define NODETHRESH 1

#define TURNSPD 100
#define DRIVESPD 200

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
volatile viveSensor V2;
int state = 0;
double xPosB, yPosB, xPosF, yPosF;
double xOldB = 0, yOldB = 0, xFiltB = 0, yFiltB = 0, xOldF = 0, yOldF = 0, xFiltF = 0, yFiltF = 0;

int leftInterruptEvenCheck = 1;
int rightInterruptEvenCheck = 1;
int middleInterruptEvenCheck = 1;

float nodes[4][2] = {{4.2, 3.8},
                    {4.5, -5.9},
                    {-6.3, -6.3},
                    {-6.3, 4.0}};

int currNode = -1;






void computePosnFromRaw() {
  // calculate the position and filter it

    //back sensor
    xPosB = tan((V1.vertAng - 90.0) * DEG_TO_RAD) * LIGHTHOUSEHEIGHT;
    yPosB = tan((V1.horzAng - 90.0) * DEG_TO_RAD) * LIGHTHOUSEHEIGHT;
    xFiltB = xOldB * 0.5 + xPosB * 0.5;
    yFiltB = yOldB * 0.5 + yPosB * 0.5;
    xOldB = xFiltB;
    yOldB = yFiltB;
//    Serial.print("back sensor: ");
//    Serial.print(xFiltB);
//    Serial.print(" ");
//    Serial.println(yFiltB);

    //front sensor
    xPosF = tan((V2.vertAng - 90.0) * DEG_TO_RAD) * LIGHTHOUSEHEIGHT;
    yPosF = tan((V2.horzAng - 90.0) * DEG_TO_RAD) * LIGHTHOUSEHEIGHT;
    xFiltF = xOldF * 0.5 + xPosF * 0.5;
    yFiltF = yOldF * 0.5 + yPosF * 0.5;
    xOldF = xFiltF;
    yOldF = yFiltF;
//    Serial.print("front sensor: ");
//    Serial.print(xFiltF);
//    Serial.print(" ");
//    Serial.println(yFiltF); 

}

int whereAmI() {
  int j = 0;
  for(int i = 0; i < 4; i++) {
      computePosnFromRaw();
      if ((xFiltF < nodes[i][j] + NODETHRESH && xFiltF > nodes[i][j] - NODETHRESH) && (yFiltF < nodes[i][j+1] + NODETHRESH && yFiltF > nodes[i][j+1] - NODETHRESH)) {
        currNode = i;
        return i;
      }
  }
  return -1;
}

void blinkLED() {
  digitalWrite(13, state);
    if (state == 1) {
      state = 0;
    }
    else {
      state = 1;
    }
}

void faceXpos() {
    while ((yFiltB > yFiltF + TURNTHRESH || yFiltB < yFiltF - TURNTHRESH) || xFiltF < xFiltB) {
      if (yFiltB > yFiltF) {
        turnLeft();
      }
      else {
        turnRight();
      }
      computePosnFromRaw();
    }
    changeSpd(0);
    faceForward();
    
}

void faceXneg() {
  while ((yFiltB > yFiltF + TURNTHRESH || yFiltB < yFiltF - TURNTHRESH) || xFiltF > xFiltB) {
    if (yFiltB > yFiltF) {
      turnRight();
    }
    else {
      turnLeft();
    }
    computePosnFromRaw();
  }
  changeSpd(0);
  faceForward();
}

void faceYpos() {
  while ((xFiltB > xFiltF + TURNTHRESH || xFiltB < xFiltF - TURNTHRESH) || yFiltF < yFiltB) {
    if (xFiltB > xFiltF) {
      turnRight();
    }
    else {
      turnLeft();
    }
    computePosnFromRaw();
  }
  changeSpd(0);
  faceForward();
}

void faceYneg() {
  while ((xFiltB > xFiltF + TURNTHRESH || xFiltB < xFiltF - TURNTHRESH) || yFiltF > yFiltB) {
    if (xFiltB > xFiltF) {
      turnLeft();
    }
    else {
      turnRight();  
    }
    computePosnFromRaw();
  }
  changeSpd(0);
  faceForward();
}

bool facingXneg() {
  if(!((yFiltB > yFiltF + FACINGTHRESH || yFiltB < yFiltF - FACINGTHRESH) || xFiltF > xFiltB)) {
    return true;
  }
  else {
    return false;
  }
}

bool facingXpos() {
  if (!((yFiltB > yFiltF + FACINGTHRESH || yFiltB < yFiltF - FACINGTHRESH) || xFiltF < xFiltB)) {
    return true;
  }
  else {
    return false;
  }
}

bool facingYpos() {
  if (!((xFiltB > xFiltF + FACINGTHRESH || xFiltB < xFiltF - FACINGTHRESH) || yFiltF < yFiltB)) {
    return true;
  }
  else {
    return false;
  }
}

bool facingYneg() {
  if (!((xFiltB > xFiltF + FACINGTHRESH || xFiltB < xFiltF - FACINGTHRESH ) || yFiltF > yFiltB)) {
    return true;
  }
  else {
    return false;
  }
}

void driveUntil(char variableCoord, char someOperator, float desiredCoord) {
  faceForward();
  if (variableCoord == 'x') {
    if (someOperator == '<') {
      while (xFiltF > desiredCoord) {
        changeSpd(DRIVESPD);
        if(!facingXneg()){
          faceXneg();
        }
        computePosnFromRaw();
      }
    }
    else if (someOperator == '>') {
      while (xFiltF < desiredCoord) {
        changeSpd(DRIVESPD);
        if (!facingXpos()) {
          faceXpos();
        }
        computePosnFromRaw();
      }
    }
  }
  else if (variableCoord == 'y') {
    if (someOperator == '<') {
      while (yFiltF > desiredCoord) {
        changeSpd(DRIVESPD);
        if(!facingYneg()){
          faceYneg();
        }
        computePosnFromRaw();
      }
    }
    else if (someOperator == '>') {
      while (yFiltF < desiredCoord) {
        changeSpd(DRIVESPD);
        if(!facingYpos()){
          faceYpos();
        }
        computePosnFromRaw();
      }
    }
  }
  delay(100);

  changeSpd(0);

}

// variables for the xbee data
char msg[100];
int msg_index = 0;
float xOpponent, yOpponent;


void changeSpd(int newSpd) {
  if (newSpd == 0) {
    analogWrite(MOTORSPD1, newSpd);
    analogWrite(MOTORSPD2, newSpd);
  }
  else {
    analogWrite(MOTORSPD1, newSpd);
    analogWrite(MOTORSPD2, newSpd + 10);
  }

}

void turnRight() {
  changeSpd(0);
  digitalWrite(MOTORDIR1, LOW);
  digitalWrite(MOTORDIR2, LOW);
  changeSpd(TURNSPD);


}

void turnLeft() {
  changeSpd(0);
  digitalWrite(MOTORDIR1, HIGH);
  digitalWrite(MOTORDIR2, HIGH);
  changeSpd(TURNSPD);

}



void faceForward() {
  digitalWrite(MOTORDIR1, HIGH);
  digitalWrite(MOTORDIR2, LOW);
}

void faceBackward() {
  digitalWrite(MOTORDIR1, LOW);
  digitalWrite(MOTORDIR2, HIGH);
}

void gotoNode(int node) {
  switch(node) {
    case 0:
      if (currNode == 1) {
        faceYpos();
        driveUntil('y', '>', nodes[0][1]);
      }
      else {
        faceXpos();
        driveUntil('x','>',nodes[0][0]);
      }
      break;
    case 1:
      if (currNode == 0) {
        faceYneg();
        driveUntil('y','<', nodes[1][1]);
      }
      else {
        faceXpos();
        driveUntil('x','>', nodes[1][0]);
      }
      break;
    case 2:
      if (currNode == 1) {
        faceXneg();
        driveUntil('x', '<', nodes[2][0]);
      }
      else {
        faceYneg();
        driveUntil('y','<',nodes[2][1]);
      }
      break;
    case 3:
      if (currNode == 0) {
        faceXneg();
        driveUntil('x', '<', nodes[3][0]);
      }
      else {
        faceYpos();
        driveUntil('y','>',nodes[3][1]);
      }
      break;
  }

}

float distance(float myX, float myY, float yourX, float yourY) {
  return sqrt((myX-yourX)*(myX-yourX) + (myY-yourY)*(myY-yourY));
}

void flee() {
  whereAmI();
  float node0Dist = distance(nodes[0][0], nodes[0][1], xOpponent, yOpponent);
  float node1Dist = distance(nodes[1][0], nodes[1][1], xOpponent, yOpponent);
  float node2Dist = distance(nodes[2][0], nodes[2][1], xOpponent, yOpponent);
  float node3Dist = distance(nodes[3][0], nodes[3][1], xOpponent, yOpponent);

  switch(currNode) {
    case 0:
      if (node1Dist > node3Dist) {
        gotoNode(1);
      }
      else {
        gotoNode(3);
      }
      break;
    case 1:
      if (node2Dist > node0Dist) {
        //go to node 2
        gotoNode(2);
      }
      else {
        //go to node 0
        gotoNode(0);
      }
      break;
    case 2:
      if (node1Dist > node3Dist) {
        //go to node 3
        gotoNode(1);
      }
      else {
        //go to node 1
        gotoNode(3);
      }
      break;
    case 3:
      if (node0Dist > node2Dist) {
        //go to node 0
        gotoNode(0);
      }
      else {
        //go to node 2
        gotoNode(2);
      }
      break;
  }
}



void setup() {
  Serial.begin(9600); // to talk to the computer
  Serial4.begin(9600); // to listen to the xbee
  pinMode(13, OUTPUT); // to blink the led on pin 13
  pinMode(V1PIN, INPUT); // to read the sensor

  // initialize the sensor variables
  //back sensor
  V1.horzAng = 0;
  V1.vertAng = 0;
  V1.useMe = 0;
  V1.collected = 0;

  //front sensor
  V2.horzAng = 0;
  V2.vertAng = 0;
  V2.useMe = 0;
  V2.collected = 0;

  
  // interrupt on any sensor change
  attachInterrupt(digitalPinToInterrupt(V1PIN), ISRV1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(V2PIN), ISRV2, CHANGE);

    //PWM Pin Setup
  pinMode(MOTORDIR2, OUTPUT);
  pinMode(MOTORDIR1, OUTPUT);
  pinMode(MOTORSPD1, OUTPUT);
  pinMode(MOTORSPD2, OUTPUT);
  digitalWrite(MOTORDIR1, HIGH);
  digitalWrite(MOTORDIR2, LOW);
  changeSpd(0);

  //setting up audio stuff
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(LEFTBUTTON), leftInterrupt, LOW);
  attachInterrupt(digitalPinToInterrupt(MIDDLEBUTTON), middleInterrupt, LOW);
  attachInterrupt(digitalPinToInterrupt(RIGHTBUTTON), rightInterrupt, LOW);
}

int time = millis();

void loop() {
  // see if the xbee has sent any data
  if (Serial4.available() > 0) {
    msg[msg_index] = Serial4.read();
    // if you get a newline, there is data to read
    if (msg[msg_index] == '\n') {
      msg_index = 0;
      // data is in the format of two floats seperated by spaces
      sscanf(msg, "%f %f", &xOpponent, &yOpponent);

//      Serial.print("op: ");
//      Serial.print(xOpponent);
//      Serial.print(" ");
//      Serial.println(yOpponent);
    }
    else {
      // did not get a newline yet, just store for later
      msg_index++;
      if (msg_index == 100) {
        msg_index = 0;
      }
    }
  }
// if ((millis() % 1000) == 0) {
//   Serial.print(V1.useMe);
//   Serial.print(" ");
//   Serial.println(V2.useMe);

// }
 
  // if the sensor data is new
  if (V1.useMe && V2.useMe) {
    V1.useMe = 0;
    V2.useMe = 0;

    computePosnFromRaw();
    // blink the led so you can tell if you are getting sensor data
    blinkLED();
    float dist = distance(xFiltF, yFiltF, xOpponent, yOpponent);
    Serial.println(dist);

    // Serial.println(whereAmI());
//    if (dist < 5.5) {
//      flee();
//    }
    faceForward();
    changeSpd(DRIVESPD);
       
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

void ISRV2() {
  // get the time the interrupt occured
  unsigned long mic = micros();
  int i;

  // shift the time into the buffer
  for (i = 0; i < 10; i++) {
    V2.changeTime[i] = V2.changeTime[i + 1];
  }
  V2.changeTime[10] = mic;

  // if the buffer is full
  if (V2.collected < 11) {
    V2.collected++;
  }
  else {
    // if the times match the waveform pattern
    if ((V2.changeTime[1] - V2.changeTime[0] > 7000) && (V2.changeTime[3] - V2.changeTime[2] > 7000) && (V2.changeTime[6] - V2.changeTime[5] < 50) && (V2.changeTime[10] - V2.changeTime[9] < 50)) {
      V2.horzAng = (V2.changeTime[5] - V2.changeTime[4]) * DEG_PER_US;
      V2.vertAng = (V2.changeTime[9] - V2.changeTime[8]) * DEG_PER_US;
      V2.useMe = 1;
    }
  }
}

void leftInterrupt() {
  // Serial.println(leftInterruptEvenCheck);
 delay(10);
 if (digitalRead(LEFTBUTTON) == LOW) {
    leftInterruptEvenCheck++;
    if (leftInterruptEvenCheck % 2 != 0) {  
      faceBackward();
      changeSpd(150);
      delay(350);
      turnRight();
      delay(150);
      faceForward();
      delay(250);
      changeSpd(0);

    }

 }


}


void rightInterrupt() {
  // Serial.println(leftInterruptEvenCheck);
 delay(10);
 if (digitalRead(RIGHTBUTTON) == LOW) {
    rightInterruptEvenCheck++;
    if (rightInterruptEvenCheck % 2 != 0) {  
        faceBackward();
        changeSpd(150);
        delay(350);
        turnLeft();
        delay(150);
        faceForward();
        delay(250);
        changeSpd(0);

    }   
 }


}

void middleInterrupt() {
 delay(10);
 if (digitalRead(MIDDLEBUTTON) == LOW) {
    middleInterruptEvenCheck++;
    if (middleInterruptEvenCheck % 2 != 0) {
      faceBackward();
      changeSpd(150);
      delay(500);
      turnRight();
      delay(1000);
      changeSpd(0);
    }
 }

}

























