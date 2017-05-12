#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>


// WAV files converted to code by wav2sketch
#include "AudioSampleR2d2_screamloud.h"
#include "AudioSampleMarchshort.h"
#include "AudioSampleB1.h"
#include "AudioSampleB2.h"
#include "AudioSampleB3.h"
#include "AudioSampleB4.h"
#include "AudioSampleB5.h"
#include "AudioSampleB6.h"
#include "AudioSampleB7.h"
#include "AudioSampleB8.h"
#include "AudioSampleCant.h"



// Create the Audio components.  These should be created in the
// order data flows, inputs/sources -> processing -> outputs
//
AudioPlayMemory    sound0;
AudioPlayMemory    sound1;  // six memory players, so we can play
AudioPlayMemory    sound2;  // all six sounds simultaneously
AudioPlayMemory    sound3;
AudioPlayMemory    sound4;
AudioPlayMemory    sound5;
AudioMixer4        mix1;    // two 4-channel mixers are needed in
AudioMixer4        mix2;    // tandem to combine 6 audio sources
AudioOutputI2S     headphones;
AudioOutputAnalog  dac0;     // play to both I2S audio board and on-chip DAC

// Create Audio connections between the components
//
AudioConnection c1(sound0, 0, mix1, 0);
AudioConnection c2(sound1, 0, mix1, 1);
AudioConnection c3(sound2, 0, mix1, 2);
AudioConnection c4(sound3, 0, mix1, 3);
AudioConnection c5(mix1, 0, mix2, 0);   // output of mix1 into 1st input on mix2
AudioConnection c6(sound4, 0, mix2, 1);
AudioConnection c7(sound5, 0, mix2, 2);
AudioConnection c8(mix2, 0, headphones, 0);
AudioConnection c9(mix2, 0, headphones, 1);
AudioConnection c10(mix2, 0, dac0, 0);

// Create an object to control the audio shield.
//
AudioControlSGTL5000 audioShield;

// Bounce objects to read six pushbuttons (pins 0-5)
//
Bounce button0 = Bounce(0, 5);
Bounce button1 = Bounce(1, 5);

void setup() {
  // Configure the pushbutton pins for pullups.
  // Each button should connect from the pin to GND.
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  Serial.begin(9600);
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(10);

  // turn on the output
  audioShield.enable();
  audioShield.volume(0.5);

  // by default the Teensy 3.1 DAC uses 3.3Vp-p output
  // if your 3.3V power has noise, switching to the
  // internal 1.2V reference can give you a clean signal
  //dac.analogReference(INTERNAL);

  // reduce the gain on mixer channels, so more than 1
  // sound can play simultaneously without clipping
  mix1.gain(0, 1);
  mix1.gain(1, 1);
  mix1.gain(2, 1);
  mix1.gain(3, 1);
  mix2.gain(1, 1);
  mix2.gain(2, 1);

  attachInterrupt(0, scream, LOW);
  attachInterrupt(1, march, LOW);
}

void loop() {
  // Update all the button objects
  button0.update();
  button1.update();
  // When the buttons are pressed, just start a sound playing.
  // The audio library will play each sound through the mixers
  // so any combination can play simultaneously.
  //
  if (button1.fallingEdge()) {
    sound0.play(AudioSampleR2d2_screamloud);
  }
  delay(2000);
  int beep = random(1, 8);
  if (!sound0.isPlaying()) {
    switch (beep) {
      case 1:
        sound0.play(AudioSampleB1);
        break;
      case 2:
        sound0.play(AudioSampleB2);
        break;
      case 3:
        sound0.play(AudioSampleB3);
        break;
      case 4:
        sound0.play(AudioSampleB4);
        break;
      case 5:
        sound0.play(AudioSampleB5);
        break;
      case 6:
        sound0.play(AudioSampleB6);
        break;
      case 7:
        sound0.play(AudioSampleB7);
        break;
      case 8:
        sound0.play(AudioSampleB8);
        break;
    }
  }
}

void scream() {
  delay(10);
 if (digitalRead(0) == LOW) {
  sound0.play(AudioSampleR2d2_screamloud);
  }
}
void march() {
  delay(10);
 if (digitalRead(1) == LOW) {
  sound0.play(AudioSampleMarchshort);
 }
}


