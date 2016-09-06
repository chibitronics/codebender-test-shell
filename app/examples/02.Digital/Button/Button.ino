//  This example is Chibitronics Love to Code tested!
/*
  Button
 
 Turns on and off a light emitting diode(LED) connected to
 pin A1, when pressing a pushbutton attached to pin A0. 
 
 
 The circuit:
 * LED attached from pin A2 to ground
 * pushbutton attached to pin A3 from ground
 * on chibitronics boards, A3 can be configured to have an intenal pull-up
 
 created 2005
 by DojoDave <http://www.0j0.org>
 modified 30 Aug 2011
 by Tom Igoe
 modified 25 May 2016
 by bunnie

 This example code is in the public domain.
 
 http://www.arduino.cc/en/Tutorial/Button
 */

// constants won't change. They're used here to 
// set pin numbers:
const int buttonPin = A0;     // the number of the pushbutton pin
const int ledPin =  A1;      // the number of the LED pin

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

void setup() {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);      
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);     
}

void loop(){
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is LOW:
  if (buttonState == LOW) {     
    // turn LED on:    
    digitalWrite(ledPin, HIGH);  
  } 
  else {
    // turn LED off:
    digitalWrite(ledPin, LOW); 
  }
}