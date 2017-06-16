//  Love to Code

// Test #3 of the LTC API

int switchPin = 0;   // pin for the switch. Connect to ground to "press" the switch
int ledPin = 5;      // LED for output

int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;  // the previous reading from the input pin

unsigned int lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned int debounceDelay = 50;    // the debounce time; increase if the switch isn't reliable

void setup() {
  pullupMode(switchPin);
  outputMode(ledPin);

  if( ledState == HIGH )
    on(ledPin);
  else
    off(ledPin);
}

void loop() {
  int reading = read(switchPin);

  if (reading != lastButtonState) {
    lastDebounceTime = time();
  }

  if ((time() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) { 
      buttonState = reading;

      if (buttonState == LOW) {
	ledState = !ledState;
      }
    }
  }

  if( ledState == HIGH )
    on(ledPin);
  else
    off(ledPin);

  // save the button's reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
}
