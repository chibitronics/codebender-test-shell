//  This example is Chibitronics Love to Code tested!
/*
  Analog Input
 Demonstrates analog input by reading an analog sensor on analog pin A2 and
 turning on and off a light emitting diode(LED)  connected to digital pin A1. 
 The amount of time the LED will be on and off depends on
 the value obtained by analogRead(). 
 
 The circuit:
 * Potentiometer attached to A2
 * one side pin (either one) to A3
 * the other side pin to A0
 * LED anode (long leg) attached to output A1
 * LED cathode (short leg) attached to ground
 
 Created by David Cuartielles
 modified 30 Aug 2011
 By Tom Igoe
 modified 25 May 2016
 by bunnie
 
 This example code is in the public domain.
 
 http://arduino.cc/en/Tutorial/AnalogInput
 
 */

int sensorPin = A2;    // select the input pin for the potentiometer
int ledPin = A1;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  pinMode(A3, OUTPUT);
  digitalWrite(A3, LOW);   // A3 is the - reference
  
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);  // A0 is the + voltage
  
  pinMode(A2, INPUT);      // A2 is the sensor input

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