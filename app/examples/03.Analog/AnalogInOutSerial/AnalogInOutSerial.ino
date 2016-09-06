//  This example is Chibitronics Love to Code tested!
/*
  Analog input, analog output, serial output
 
 Reads an analog input pin, maps the result to a range from 0 to 255
 and uses the result to set the pulsewidth modulation (PWM) of an output pin.
 Also prints the results to the serial monitor.
 
 The circuit:
 * potentiometer connected to pin A2.
   Center pin of the potentiometer goes to the analog pin.
   side pins of the potentiometer go to A0 and A3
 * LED connected from digital pin A1 to ground
 
 created 29 Dec. 2008
 modified 9 Apr 2012
 by Tom Igoe
 modified 25 May 2016
 by bunnie
 
 This example code is in the public domain.
 
 */

// These constants won't change.  They're used to give names
// to the pins used:
const int analogInPin = A2;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = A1; // Analog output pin that the LED is attached to

int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)

void setup() {
  Serial.begin(9600); 

  pinMode(A3, OUTPUT);
  digitalWrite(A3, LOW);   // A3 is the - reference
  
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);  // A0 is the + voltage
  
  pinMode(A2, INPUT);      // A2 is the sensor input

  pinMode(A1, OUTPUT);     // A1 has the LED

}

void loop() {
  // read the analog in value:
  sensorValue = analogRead(analogInPin);            
  // map it to the range of the analog out:
  outputValue = map(sensorValue, 0, 1023, 0, 255);  
  // change the analog out value:
  analogWrite(analogOutPin, outputValue);           

  // print the results to the serial monitor:
  Serial.print("sensor = " );                       
  Serial.print(sensorValue);      
  Serial.print("\t output = ");      
  Serial.println(outputValue);   

  // wait 2 milliseconds before the next loop
  // for the analog-to-digital converter to settle
  // after the last reading:
  delay(2);                     
}
