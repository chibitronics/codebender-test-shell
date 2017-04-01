//  This example is Chibitronics Love to Code tested!
/*
 Resistive touch

 This example configures all the pins as inputs and waits for a touch
 stimulus from the pin to ground.

 Based on the touch detected, the NeoPixel LED will flash the number
 of times equal to the port number.

 25 May 2016
 by bunnie

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/Debounce
 */

#include "Adafruit_NeoPixel.h"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_BUILTIN_RGB,
                                            NEO_GRB + NEO_KHZ800);


// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

int pinState[6];
int pinFocus = -2;  // the pin we're currently focused on in the debounce loop
int pinHeld = 0;

void setup() {
  int i;
  for( i = 0; i < 4; i++ ) {
    pinMode(A0 + i, INPUT);
  }
  for( i = 0; i < 2; i++ ) {
    pinMode(D0 + i, INPUT);
  }
  for( i = 0; i < 6; i++ ) {
    pinState[i] = HIGH;          // pins default to high
  }

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  Serial.begin(9600);
}

void updatePinState() {
  int i;
  for( i = 0; i < 4; i++ ) {
    pinState[i] = digitalRead(A0 + i);
  }
  for( i = 0; i < 2; i++ ) {
    pinState[4 + i] = digitalRead(D0 + i);
  }
}

// returns the lowest-number pin that is currently pressed
int whichPinPressed() {
  int i;

  for( i = 0; i < 6; i++ ) {
    if( pinState[i] != HIGH )
      return i;
  }

  // if none changed, return -1
  return -1;
}

void loop() {
  int i;
  int whichPin;

  // function to read all pins and copy to an array
  updatePinState();

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  whichPin = whichPinPressed();
  Serial.println(whichPin);  // a little debug info
  if ( (whichPin != -1) && (pinFocus == -2)) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    pinFocus = whichPin;
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // if the current pin we're looking at is still the focus, then we've got a hit
    if( (whichPin == pinFocus) ) {
      if( pinHeld == 0 ) {
       // blink out the pin that was hit
       for( i = 0; i < pinFocus + 1; i++ ) {
         strip.setPixelColor(0, strip.Color(31,3,18));
         strip.show();
         delay(200);
         strip.setPixelColor(0, strip.Color(0,0,0));
         strip.show();
         delay(200);
       }
       pinHeld = 1;
      } else {
       // if pin is being held, do nothing until it's released...
      }
    } else {
      if( whichPin == -1 ) {
       pinFocus = -2;  // we're not looking at /any/ pin, reset pinHeld state
       pinHeld = 0;
      } else {
       // in this case we rolled over to another pin...need to let go of all pins before we can move on!
      }
    }
  }
}

