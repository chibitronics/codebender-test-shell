// Love to Code

// Fade example!
// This example shows how to fade an LED.

// Humans think in decimal; Computers think in binary. So:
// When picking a brightness, a human might prefer a scale of 1-100
// When picking a brightness, a computer might prefer a scale of 0-255

// The Chibi Chip is a computer, so we tell it brightnesses on a scale of 0-255.
 
int led = 5;          // you can select a pin: 0, 1, 2, 3, 4 or 5
int fadeAmount = 5;   // sets how quickly we fade in and out. Change it to 10 and see what happens!

// the setup routine runs once when you press reset:
void setup()  { 
  pinMode(led, OUTPUT);
} 

int brightness = 0;   // keep track of the LED's brightness

// the loop routine runs over and over again forever:
void loop()  {
  analogWrite(led, brightness);    // update the led's brightness

  // move the brightness by fadeAmount toward a brightness limit
  brightness = brightness + fadeAmount;

  // check if the brightness is at the bottom or top of the 0-255 scale
  // if it is, invert the amount we fade by
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount ; 
  }     
  // pause for 30 milliseconds, or else the effect is too fast
  delay(30);                            
}
