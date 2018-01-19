// Love to Code

// Wave on, Wave off light switch
//
// Waving your hand across the LTC Light Sensor will flip the light
// on pin 0 on and ofF!

// To run the demo, connect the LTC Light Sensor to pin 5. If you have
// the Chibi Clip accessory, clamping the sensor into pin 5 will also
// line up GND and +3V perfectly for this demo.
//
// The light sensor will output a value from 0 (perfectly dark) to
// 1023 (direct sunlight). A typical office environment will have
// an ambient light level around 300; if your lighting is dim,
// you may need to adjust your light threshold. 

// Pick which pins to use as sensor or LED (a number from 0-5):
const int sensorPin = 5;   // pin for the switch. Connect to ground to "press" the switch
const int ledPin = 0;      // LED for output

const int lightThreshold = 200; // a number from 0-1023 represting the light/dark threshold.
// You may need to adjust the threshold based on the ambient light condition in your room.

// These variables let the computer keep track of the light state
int ledState = HIGH;         // the current state of the output pin
int lightState;             // the current reading from the input pin
int lastLightState = HIGH;  // the previous reading from the input pin

// These variables let the computer keep track of time. Time can't be negative, so it's an "unsigned int"
unsigned int lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned int debounceDelay = 50;    // the debounce time, makes the light sensor ignore short transitions or partial shadows

void setup() {
  pinMode(sensorPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // set initial LED state
  digitalWrite(ledPin, ledState);
}

void loop() {
  int reading;
  
  int sensorValue = analogRead(sensorPin);

  if( sensorValue > lightThreshold ) {
    reading = HIGH;
  } else {
    reading = LOW;
  }

  // Whenever we see the light level go past a threshold, take note of the time
  if (reading != lastLightState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  // after a short pause, check to see if the reading has changed
  if ((millis() - lastDebounceTime) > debounceDelay) {

    // if after the delay, the reading is still changed, we've passed the bounce period
    if (reading != lightState) { 
      lightState = reading;

      // only flip the LED if the new light state is LOW
      if (lightState == LOW) {
	ledState = !ledState;
      }
    }
  }

  // update the LED:
  digitalWrite(ledPin, ledState);

  // save the light's reading.  Next time through the loop,
  // it'll be the lastLightState:
  lastLightState = reading;
}
