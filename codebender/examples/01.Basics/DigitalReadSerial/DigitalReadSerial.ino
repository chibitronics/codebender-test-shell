//  This example is Chibitronics Love to Code tested!
/*
  DigitalReadSerial
 Reads a digital input on pin A3, prints the result to the serial monitor 
 
 This example code is in the public domain.
 */

// pin A3 has a pushbutton attached to it. Give it a name:
int pushButton = A3;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(115200);  // currently, the only speed supported
  // make the pushbutton's pin an input:
  pinMode(pushButton, INPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input pin:
  int buttonState = digitalRead(pushButton);
  // print out the state of the button:
  Serial.println(buttonState);
  delay(1);        // delay in between reads for stability
}



