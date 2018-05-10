/* "React" two-player game
 *   When the color LED turns white, be the first person to press your
 * button!  The Green player gets button 0, and the Red player gets
 * button 5.  Whoever hits their button first wins, and the color LED
 * will turn the color of the winner.
 *   It is recommended that you design a board with paper switches to
 * connect pins 0 and 5 to +3.3V when the button is pressed, and color
 * the switches to match the player's color.
 *
 * by Kent Cross, 2018-01-11
 */

#include "Adafruit_NeoPixel.h"	// Library for interfacing with the Adafruit NeoPixel LED
#include "html_colors.h" 		// add this line to use color names instead of codes

#define DELAY_COLOR     COLOR_BLACK
#define GO_COLOR        COLOR_WHITE
#define PLAYER1_COLOR   COLOR_GREEN
#define PLAYER2_COLOR   COLOR_RED

// Define the NewPixel strip class
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_BUILTIN_RGB, NEO_GRB + NEO_KHZ800);

static long seed = 0;

void setup() {
  // Initialize the NeoPixel LED
  strip.begin();
  strip.setBrightness(32);

  // Set up the initial Random Number sequence
  // Note: Since seed is a constant, the random sequence will always be the same.
  // This is why we reset the sequence below based on how long it takes player to press first button
  randomSeed(seed);
}

void loop() {
  neoPixelOn(DELAY_COLOR);		// Turn NeoPixel Off
  pause(random(1000,5000));		// Wait from 1 to 5 seconds to add suspense
  neoPixelOn(GO_COLOR);		    // Turn NewPixel On
  int key = readKey0or5();		// Read who reacts first
  if (key == 0)					// Turn NeoPixel to color of person who reacted first
    neoPixelOn(PLAYER1_COLOR);
  else
    neoPixelOn(PLAYER2_COLOR);
  pause(1000);					// Let them savor their vistory for a second
  if (seed == 0)				// If this first time through game, reset random sequence 
    seed = millis();			// based on time played so far
  randomSeed(seed);
}

// turn the NeoPixel on to a specific color
void neoPixelOn(int color) {
  strip.setPixelColor(0, color); 
  strip.show();
}

// return if key 0 or 5 pressed first
int readKey0or5() {

  // Set key 0 and 5 to input mode
  inputMode(0);
  inputMode(5);
  
  // loop until a key is pressed
  int pressed = 0;
  int key = 0;
  while (pressed == 0) {
    pressed = read(key);	// Check to see if this particular key is pressed
    if (pressed == 0) {		// if not, then check the other key
      if (key == 0)
        key = 5;
      else
        key = 0;
    }
  }
  return(key);				// Return which key was pressed first
}