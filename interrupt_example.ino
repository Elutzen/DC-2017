#define BUTTONPIN 2;
#define buttonState 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(BUTTONPIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN),wall,CHANGE);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void wall(){
  //turn the fuck around
}

