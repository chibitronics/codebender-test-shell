/*
  ReadAnalogVoltage
  Reads an analog input on pin A1, converts it to voltage, and prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A1, and the outside pins to A2 and A0.
 
 This example code is in the public domain.
 */

// the setup routine runs once when you press reset:
int calibrationValue;
void setup() {
  // the chibitronics implementation always runs at 115200 regardless of the argument
  Serial.begin(115200);

  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW);   // A0 is the - reference
  
  pinMode(A2, OUTPUT);
  digitalWrite(A2, HIGH);  // A2 is the + voltage
  
  pinMode(A1, INPUT);      // A1 is the sensor input

  pinMode(A6, INPUT);      // A6 is the "bandgap" reference, it is stable at 1V over temperature and different chips

  // use A6 bandgap value to calibrate our voltage!
  calibrationValue = analogRead(A6);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A1);
  // Convert the analog reading (which goes from 0 - 4096) to a voltage (0 - 3.3V):
  float voltage = (sensorValue / calibrationValue) * 1.0;
  // print out the value you read:
  Serial.println(voltage);
}
