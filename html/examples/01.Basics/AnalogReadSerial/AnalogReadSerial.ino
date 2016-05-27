//  This example is Chibitronics Love to Code tested!
/*
  AnalogReadSerial
  Reads an analog input on pin 1, prints the result to the serial monitor.

  Attach the center pin of a potentiometer to pin A1, 
  and the outside pins to A0 (set to gnd) and A2 (set to VMCU).

 This example code is in the public domain.
 */

// the setup routine runs once when you press reset:
void setup() {
  // the chibitronics implementation always runs at 115200 regardless of the argument
  Serial.begin(115200);

  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW);   // A0 is the - reference
  
  pinMode(A2, OUTPUT);
  digitalWrite(A2, HIGH);  // A2 is the + voltage
  
  pinMode(A1, INPUT);      // A1 is the sensor input
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 1:
  int sensorValue = analogRead(A1);

  // print out the value you read:
  Serial.println(sensorValue);

  delay(1);        // delay in between reads for stability
}
