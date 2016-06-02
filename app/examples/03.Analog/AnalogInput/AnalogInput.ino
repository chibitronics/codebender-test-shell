//  This example is Chibitronics Love to Code tested!
/*
  Analog Input
 Demonstrates analog input by reading an analog sensor on analog pin A1 and
 turning on and off a light emitting diode(LED)  connected to digital pin A2. 
 The amount of time the LED will be on and off depends on
 the value obtained by analogRead(). 
 
 The circuit:
 * Potentiometer attached to A1
 * one side pin (either one) to A0
 * the other side pin to A3
 * LED anode (long leg) attached to output A2
 * LED cathode (short leg) attached to ground
 
 Created by David Cuartielles
 modified 30 Aug 2011
 By Tom Igoe
 modified 25 May 2016
 by bunnie
 
 This example code is in the public domain.
 
 http://arduino.cc/en/Tutorial/AnalogInput
 
 */

int sensorPin = A1;    // select the input pin for the potentiometer
int ledPin = A2;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW);   // A0 is the - reference
  
  pinMode(A3, OUTPUT);
  digitalWrite(A3, HIGH);  // A3 is the + voltage
  
  pinMode(A1, INPUT);      // A1 is the sensor input

  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);  
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);    
  // turn the ledPin on
  digitalWrite(ledPin, HIGH);  
  // stop the program for <sensorValue> milliseconds:
  delay(sensorValue);          
  // turn the ledPin off:        
  digitalWrite(ledPin, LOW);   
  // stop the program for for <sensorValue> milliseconds:
  delay(sensorValue);                  
}