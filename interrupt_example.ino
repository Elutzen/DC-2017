#define BUTTONPIN 14
#define LED 13


void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTONPIN, INPUT);
  pinMode(LED, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN),wall,CHANGE);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void wall(){
  //turn the fuck around
  int buttonState = digitalRead(BUTTONPIN);
  digitalWrite(LED, buttonState);
}

}

