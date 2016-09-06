//  This example is Chibitronics Love to Code tested!
/*
  Calibration
 
 Demonstrates one technique for calibrating sensor input.  The
 sensor readings during the first five seconds of the sketch
 execution define the minimum and maximum of expected values
 attached to the sensor pin.
 
 The sensor minimum and maximum initial values may seem backwards.
 Initially, you set the minimum high and listen for anything 
 lower, saving it as the new minimum. Likewise, you set the
 maximum low and listen for anything higher as the new maximum.
 
 The circuit:
 * Analog sensor (potentiometer will do) attached to analog input A1 (A3 & A0 are gnd and VMCU)
 * LED attached from digital pin A1 to ground
 
 created 29 Oct 2008
 By David A Mellis
 modified 30 Aug 2011
 By Tom Igoe
 modified 25 May 2016
 by bunnie
 
 http://arduino.cc/en/Tutorial/Calibration
 
 This example code is in the public domain.
 
 */

// These constants won't change:
const int sensorPin = A2;    // pin that the sensor is attached to
const int ledPin = A1;       // pin that the LED is attached to

// variables:
int sensorValue = 0;         // the sensor value
int sensorMin = 4095;        // minimum sensor value
int sensorMax = 0;           // maximum sensor value


void setup() {
  pinMode(A3, OUTPUT);
  digitalWrite(A3, LOW);   // A3 is the - reference
  
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);  // A0 is the + voltage
  
  pinMode(A2, INPUT);      // A2 is the sensor input

  // turn on LED to signal the start of the calibration period:
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  // calibrate during the first five seconds 
  while (millis() < 5000) {
    sensorValue = analogRead(sensorPin);

    // record the maximum sensor value
    if (sensorValue > sensorMax) {
      sensorMax = sensorValue;
    }

    // record the minimum sensor value
    if (sensorValue < sensorMin) {
      sensorMin = sensorValue;
    }
  }

  // signal the end of the calibration period
  digitalWrite(ledPin, LOW);
}

void loop() {
  // read the sensor:
  sensorValue = analogRead(sensorPin);

  // apply the calibration to the sensor reading
  sensorValue = map(sensorValue, sensorMin, sensorMax, 0, 255);

  // in case the sensor value is outside the range seen during calibration
  sensorValue = constrain(sensorValue, 0, 255);

  // fade the LED using the calibrated value:
  analogWrite(ledPin, sensorValue);
}
