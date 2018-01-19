// Love to Code
// Add-on #1: Light-O-Meter

int lightLevel = 0;
  
void setup() {
  outputMode(0);  // pin 0 turns on and off the LED
  inputMode(5);   // pin 5 connects to our light sensor
}

void loop() {
    lightLevel = readLevel(5);  // read light level
if(lightLevel > 0) {   // if level is greater than 0,
    on(0);             // turn on pin 0
  } else {
    off(0);            // otherwise turn off pin 0
  }
if(lightLevel > 20) {  // if level is greater than 20,
    on(1);             // turn on pin 1
  } else {
    off(1);            // otherwise turn off pin 1
  }
if(lightLevel > 40) {  // if level is greater than 40,
    on(2);             // turn on pin 2
  } else {
    off(2);            // otherwise turn off pin 2
  }
if(lightLevel > 60) {  // if level is greater than 60
    on(3);             // turn on pin 3
  } else {
    off(3);            // otherwise turn off pin 3
  }
if(lightLevel > 80) {  // if level is greater than 80,
  on(4);               // turn on pin 4   
  } else {
  off(4);              // otherwise turn off pin 4
  }
}
 