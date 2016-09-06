//  This example is Chibitronics Love to Code tested!
/*
 Input Pullup Serial
 
 This example demonstrates the use of pinMode(INPUT_PULLUP). It reads a 
 digital input on pin 2 and prints the results to the serial monitor.
 
 The circuit: 
 * Momentary switch attached from pin A0 to ground 
 * Built-in LED on pin A1
 
 Unlike pinMode(INPUT), there is no pull-down resistor necessary. An internal 
 20K-ohm resistor is pulled to 5V. This configuration causes the input to 
 read HIGH when the switch is open, and LOW when it is closed. 
 
 created 14 March 2012
 by Scott Fitzgerald
 modified 25 May 2016
 by bunnie
 
 http://www.arduino.cc/en/Tutorial/InputPullupSerial
 
 This example code is in the public domain
 
 */

void setup(){
  //start serial connection
  Serial.begin(9600);
  //configure A0 as an input and enable the internal pull-up resistor
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, OUTPUT); 

}

void loop(){
  //read the pushbutton value into a variable
  int sensorVal = digitalRead(A0);
  //print out the value of the pushbutton
  Serial.println(sensorVal);
  
  // Keep in mind the pullup means the pushbutton's
  // logic is inverted. It goes HIGH when it's open,
  // and LOW when it's pressed. Turn on pin A1 when the 
  // button's pressed, and off when it's not:
  if (sensorVal == HIGH) {
    digitalWrite(A1, LOW);
  } 
  else {
    digitalWrite(A1, HIGH);
  }
}



