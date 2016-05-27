//  This example is Chibitronics Love to Code tested!
/*
  Pitch follower
 
 Plays a pitch that changes based on a changing analog input
 
 circuit:
 * 8-ohm speaker on A3
 pick one of:
 * photoresistor on A1 to A2 (set to VMCU by code)
 * 4.7K resistor on A1 to A0 (set to ground by code)
 -OR-
 * potentiometer with middle terminal to A1, and side terminals to A0 and A2
 
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
  Serial.begin(115200);

  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW);   // A0 is the - reference
  
  pinMode(A2, OUTPUT);
  digitalWrite(A2, HIGH);  // A2 is the + voltage
  
  pinMode(A1, INPUT);      // A1 is the sensor input
}

void loop() {
  // read the sensor:
  int sensorReading = analogRead(A1);
  // print the sensor reading so you know its range
  Serial.println(sensorReading);
  // map the analog input range (in this case, 400 - 1000 from the photoresistor)
  // to the output pitch range (120 - 1500Hz)
  // change the minimum and maximum input numbers below
  // depending on the range your sensor's giving:
  int thisPitch = map(sensorReading, 400, 1000, 120, 1500);

  // play the pitch:
  tone(A3, thisPitch, 10);
  delay(1);        // delay in between reads for stability
}






