// Love to Code

// Makey-Makey style resistive touch to keyboard output demo.

// Note that on Love to Code, you need to touch and hold +3V and press
// a pin, unlike Makey Makey where you hold ground and press a pin.

#include "Keyboard.h"

// Setup your key mapping here. Each line corresponds to a key,
// from pin 0 through 5 on the Chibi Chip
// The below mapping will make pins 0-3 press the keys 0-3, while
// pin 4 is the space bar and pin 5 is enter
static const char keys[6] = {
  '0',
  '1',
  '2',
  '3',
  ' ',  // space key
  KEY_RETURN,
};

/* 
// example of using the special key codes below. Don't use the quote character with them!
// The below mapping will make pins 0-3 be left, up, down, and right arrows respectively, while
// pin 4 is the space bar and pin 5 is enter
static const char keys[6] = {
  KEY_LEFT_ARROW,
  KEY_UP_ARROW,
  KEY_DOWN_ARROW,
  KEY_RIGHT_ARROW,
  ' ',  // space key
  KEY_RETURN,
};
*/

/* some more possible key codes:
KEY_UP_ARROW
KEY_DOWN_ARROW
KEY_LEFT_ARROW
KEY_RIGHT_ARROW

KEY_LEFT_CTRL
KEY_LEFT_SHIFT		
KEY_LEFT_ALT		
KEY_LEFT_GUI		
KEY_RIGHT_CTRL		
KEY_RIGHT_SHIFT		
KEY_RIGHT_ALT	
KEY_RIGHT_GUI		
KEY_BACKSPACE		
KEY_TAB				
KEY_RETURN			
KEY_ESC				
KEY_INSERT			
KEY_DELETE			
KEY_PAGE_UP			
KEY_PAGE_DOWN		
KEY_HOME
KEY_END				
KEY_CAPS_LOCK	
	
KEY_F1				
KEY_F2				
KEY_F3				
KEY_F4				
KEY_F5				
KEY_F6				
KEY_F7				
KEY_F8				
KEY_F9				
KEY_F10
KEY_F11				
KEY_F12	

*/

long lastDebounceTime = 0; 
long debounceDelay = 50;   

int pinFocus = -2;  // the pin we're currently focused on in the debounce loop
int pinHeld = 0;
int lastFocus = 0;

int pinState[6];

void setup(void) {
  unsigned int i;

  for (i = 0; i < 6; i++) {
    pinMode(i, INPUT);
    pinState[i] = LOW;          // pins default to low
  }
  Keyboard.begin();
}

void updatePinState(void) {
  unsigned int i;

  for (i = 0; i < 6; i++) {
    pinState[i] = digitalRead(i);
  }
}

// returns the lowest-number pin that is currently pressed
int whichPinPressed(void) {
  unsigned int i;

  for (i = 0; i < 6; i++) {
    if (pinState[i] == HIGH)
      return i;
  }

  // if none changed, return -1
  return -1;
}

void loop(void) {
  int whichPin;
  // function to read all pins and copy to an array
  updatePinState();

  whichPin = whichPinPressed();
  if ( (whichPin != -1) && (pinFocus == -2) ) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    pinFocus = whichPin;
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // if the current pin we're looking at is still the focus, then we've got a hit
    if (whichPin == pinFocus) {
      if (pinHeld == 0) {
        // press the key on the USB keyboard
        Keyboard.press(keys[pinFocus]);
        delay(10);
        pinHeld = 1;
        lastFocus = pinFocus;
      }
      else {
       // if pin is being held, do nothing until it's released...
      }	
    } else {
      if (whichPin == -1) {
        Keyboard.releaseAll();
        pinFocus = -2;  // we're not looking at /any/ pin, reset pinHeld state
        pinHeld = 0;
      }
      else {
        // in this case we rolled over to another pin...
	// release the last key pressed
        Keyboard.release(keys[lastFocus]);
	
        // restart the debounce code path
        pinHeld = 0;
        pinFocus = whichPin;
        lastDebounceTime = millis();
      }
    }
  }
}

