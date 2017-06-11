//  Love to Code

// Push on, Push off switch
// Connect pin 0 to ground and make the light on pin 5 turn on or off!

// You can make a switch by folding copper tape over so it touches another piece of copper tape.
// So for this example, you can make a switch as follows:
//    Connect a straight 3" run of copper tape to the "switchPin" (0 by default)
//    Connect a straight 3" run copper tape to the "GND" pin
//    Cut off the excess paper, so that the unconnected copper tape ends are at the edge of the paper.
//    Cut a 1" slit between the two pieces of copper tape
//    Cut another 1" slit to the right of the "switchPin" copper tape.
//      You will now have a dangling tab with tape on it.
//    Fold the tab over so the switchPin copper tape is facing the GND pin copper tape.
//    Press the tab to close the switch!

// All mechanical switches -- from the pushbutton on your phone to the pushbutton you just made --
// have a property called "bounce". When you push the contacts together, they don't come together
// and stop; they actually bounce off each other. The bounce is so fast humans can't notice it,
// but a computer can. This push on, push off switch example instructs the Chibi Chip how to
// ignore the initial bounce through a process called "debouncing". 

// Pick which pins to use as switch or LED (a number from 0-5):
const int switchPin = 0;   // pin for the switch. Connect to ground to "press" the switch
const int ledPin = 5;      // LED for output

// These variables let the computer keep track of the button state
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;  // the previous reading from the input pin

// These variables let the computer keep track of time. Time can't be negative, so it's an "unsigned int"
unsigned int lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned int debounceDelay = 50;    // the debounce time; increase if the switch isn't reliable

void setup() {
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  // set initial LED state
  digitalWrite(ledPin, ledState);
}

void loop() {
  int reading = digitalRead(switchPin);   // read the state of the switch into a local variable

  // Whenever we see the switch change, take note of the time
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  // after a short pause, check to see if the reading has changed
  if ((millis() - lastDebounceTime) > debounceDelay) {

    // if after the delay, the reading is still changed, we've passed the bounce period
    if (reading != buttonState) { 
      buttonState = reading;

      // only flip the LED if the new button state is LOW
      if (buttonState == LOW) {
	ledState = !ledState;
      }
    }
  }

  // update the LED:
  digitalWrite(ledPin, ledState);

  // save the button's reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
}

// Derived from http://www.arduino.cc/en/Tutorial/Debounce (public domain)

