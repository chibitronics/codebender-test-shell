//  This example is Chibitronics Love to Code tested!
/*

  Smoothing

  Reads repeatedly from an analog input, calculating a running average
  and printing it to the computer.  Keeps ten readings in an array and 
  continually averages them.
  
  The circuit:
    * Analog sensor (potentiometer will do) attached to analog input A1
    * A0 and A2 are available as ground and VMCU, respectively

  Created 22 April 2007
  By David A. Mellis  <dam@mellis.org>
  modified 9 Apr 2012
  by Tom Igoe
  modified 25 May 2016
  by bunnie

  http://www.arduino.cc/en/Tutorial/Smoothing
  
  This example code is in the public domain.


*/


// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.
const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

int inputPin = A1;

void setup()
{
  pinMode(A0, OUTPUT);
  digitalWrite(A0, LOW);   // A0 is the - reference
  
  pinMode(A2, OUTPUT);
  digitalWrite(A2, HIGH);  // A2 is the + voltage
  
  pinMode(inputPin, INPUT);      // A1 is the sensor input

  // initialize serial communication with computer:
  Serial.begin(115200);                   
  // initialize all the readings to 0: 
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;          
}

void loop() {
  // subtract the last reading:
  total= total - readings[index];         
  // read from the sensor:  
  readings[index] = analogRead(inputPin); 
  // add the reading to the total:
  total= total + readings[index];       
  // advance to the next position in the array:  
  index = index + 1;                    

  // if we're at the end of the array...
  if (index >= numReadings)              
    // ...wrap around to the beginning: 
    index = 0;                           

  // calculate the average:
  average = total / numReadings;         
  // send it to the computer as ASCII digits
  Serial.println(average);   
  delay(1);        // delay in between reads for stability            
}


