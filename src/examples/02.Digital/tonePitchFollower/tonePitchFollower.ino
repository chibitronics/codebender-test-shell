//  This example is Chibitronics Love to Code tested!
/*
  Pitch follower
 
 Plays a pitch that changes based on a changing analog input
 
 circuit:
 * 8-ohm speaker on A0
 pick one of:
 * photoresistor on A2 to A1 (set to VMCU by code)
 * 4.7K resistor on A2 to A3 (set to ground by code)
 -OR-
 * potentiometer with middle terminal to A2, and side terminals to A3 and A1
 
 created 21 Jan 2010
 modified 31 May 2012
 by Tom Igoe, with suggestion from Michael Flynn
 modified 25 May 2016
 by bunnie

This example code is in the public domain.
 
 http://arduino.cc/en/Tutorial/Tone2
 
 */


void setup() {
  // initialize serial communications (for debugging only):
  Serial.begin(9600);

  pinMode(A3, OUTPUT);
  digitalWrite(A3, LOW);   // A3 is the - reference
  
  pinMode(A1, OUTPUT);
  digitalWrite(A1, HIGH);  // A1 is the + voltage
  
  pinMode(A2, INPUT);      // A2 is the sensor input
}

void loop() {
  // read the sensor:
  int sensorReading = analogRead(A2);
  // print the sensor reading so you know its range
  Serial.println(sensorReading);
  // map the analog input range (in this case, 400 - 1000 from the photoresistor)
  // to the output pitch range (120 - 1500Hz)
  // change the minimum and maximum input numbers below
  // depending on the range your sensor's giving:
  int thisPitch = map(sensorReading, 400, 1000, 120, 1500);

  // play the pitch:
  tone(A0, thisPitch, 10);
  delay(1);        // delay in between reads for stability
}






