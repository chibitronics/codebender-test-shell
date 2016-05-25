//  This example is Chibitronics Love to Code tested!
#include "Adafruit_NeoPixel.h"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_BUILTIN_RGB,
                                            NEO_GRB + NEO_KHZ800);
/*
  Reading a serial ASCII-encoded string.
 
 This sketch demonstrates the Serial parseInt() function.
 It looks for an ASCII string of comma-separated values.
 It parses them into ints, and uses those to fade an RGB LED.
 
 Circuit: Common-anode RGB LED wired like so:
 * Red cathode: digital pin 3
 * Green cathode: digital pin 5
 * blue cathode: digital pin 6
 * anode: +5V
 
 created 13 Apr 2012
 by Tom Igoe
 
 This example code is in the public domain.
 */

void setup() {
  // initialize serial:
  Serial.begin(115200);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

}

void loop() {
  // if there's any serial available, read it:
  while (Serial.available() > 0) {

    // look for the next valid integer in the incoming serial stream:
    int red = Serial.parseInt(); 
    // do it again:
    int green = Serial.parseInt(); 
    // do it again:
    int blue = Serial.parseInt(); 

    // look for the newline. That's the end of your
    // sentence:
    int inChar = Serial.read();
    if (inChar == '\n' || inChar == '\r') {
      // use the on-board neopixel
      strip.setPixelColor(0, strip.Color(red, green, blue));
      strip.show();

      // print the three numbers in one string as hexadecimal:
      Serial.print(red, HEX);
      Serial.print(green, HEX);
      Serial.println(blue, HEX);
    }
  }
}








