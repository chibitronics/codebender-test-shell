/* "Simon Says" game
 *   The Chibi Chip will flash a series of patterns on the color LED.
 * This color corresponds to one of the input pads.  The player must
 * press the corresponding pad to continue.
 *   For example, if the color LED flashes maroon and then red, the
 * player would need to press pad 4 and then pad 1.  The order of the
 * colors is defined by the order of the pixel_color array.
 *   The maximum length of the game is defined by the MAXSEQUENCE value.
 *
 * by Kent Cross, 2018-01-11
 */

#include "Adafruit_NeoPixel.h"	// Library for interfacing with the Adafruit NeoPixel LED
#include "html_colors.h" 		// add this line to use color names instead of codes

// The highest-numbered light on the Chibi Chip hardware
#define MAXLIGHT 5

// Define the longest possible game length
#define MAXSEQUENCE 6	

// Define the various color LED pads.  The order of pixel_color determines
// the corresponding pad numbers.
unsigned int pixel_color[] = {
  COLOR_WHITE,          // Pad 0
  COLOR_RED,            // Pad 1
  COLOR_GREEN,          // Pad 2
  COLOR_PLUM,           // Pad 3
  COLOR_MAROON          // Pad 4
};

static int sequence[MAXSEQUENCE];
static long seed = 0;

// Define the NewPixel strip class
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_BUILTIN_RGB, NEO_GRB + NEO_KHZ800);

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
  
  setSequence(0);				// define the light sequence order
  neoPixelOn(COLOR_WHITE);		// Set the NeoPixel to white initially
  
  // main player loop. 
  int win = 0;
  for (int numLights=0; numLights<MAXSEQUENCE; numLights++) {
    writeOutput(numLights);		// display the sequence of lights
    win = readInput(numLights);	// get player's repeat of the sequence. win=1 if player got it wrong
    if (seed == 0) {			// if this is the first time through game, set the random number
       setSequence(1);			// seed based on how long it takes player to press fiest key
    }
    pause(500);
    if (win == 1) {				// If player got sequence wrong, restart without blinly light reward
      break;
    }
  }
  
  // If player successfully repeats the entire sequence, reward them with blinky lights
  if (win == 0) {
    blinkNeoPixel();
  }
  
}

// establish the sequence of lights to display
void setSequence(int startNum) {
  
  // If it is the first time, then re-set the random number seed based on  
  // how long it took to click first light
  if (startNum==1) {	
    seed = millis();
    randomSeed(seed);
  }
  
  // establish random sequence 
  // Note: for 1st game the first entry has already been entered, so don't reset it
  for (int seqNum=startNum; seqNum<MAXSEQUENCE; seqNum++) {
     sequence[seqNum] = random(0,5);
  }
}

// Write out a specific number of the sequence
void writeOutput(int count)
{
  // Output the sequence
  for (int light=0; light<=count; light++) {
    lightOnOff(sequence[light]);
  }
}

// Read in the input and display GREEN if gets it right or RED of gets it wrong
int readInput(int count)
{
  int key;
  for (int light=0; light<=count; light++){
    key = readKey();
    pause(100);
    if (key == sequence[light]) {
      neoPixelOn(COLOR_GREEN);
    }
    else {
      neoPixelOn(COLOR_RED);
      return(1);
    }
  }
  return(0);
}

// return a key number when it is pressed
int readKey() 
{
  // Set all keys to input mode
  for (int light=0; light <= MAXLIGHT; light++){
    inputMode(light);
  }
  
  // loop until a key is pressed
  int pressed = 0;
  int key = 0;
  while (pressed == 0) {
    pressed = read(key);	// Check to see if this particular key is pressed
    if (pressed == 0) {		// if not, then increment to check the next key
      key++;
      if (key > MAXLIGHT) {		// If checked all keys, loop back to check the first
        key = 0;
      }
    }
  }
  return(key);		// Return which key was pressed
}

// Turn a specific light on then off
void lightOnOff(int light)
{
  outputMode(light);
  on(light);
  pause(1000);
  off(light);
  pause(500);
}


// turn the NeoPixel on to a specific color
void neoPixelOn(int color)
{
  strip.setPixelColor(0, color); 
  strip.show();
  pause(1000);
}

// blink the NeoPixel as award for sucessfully playing game
void blinkNeoPixel()
{
  for (int i=0; i < (sizeof(pixel_color)/sizeof(*pixel_color)); i++) {
    strip.setPixelColor(0, pixel_color[i]); 
    strip.show();
    pause(1000);
  }
}