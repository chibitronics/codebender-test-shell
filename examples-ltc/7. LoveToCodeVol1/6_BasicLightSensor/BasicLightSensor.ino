// Love to Code
// Volume 1: Basic Light Sensor

// This code reads a light sensor on pin 5. 
// Pin 0 turns ON when it is dark and OFF when it is bright.

int lightLevel = 0;  // make a variable named lightLevel to save current light level

void setup(){
  outputMode(0);  // LED on pin 0 will turn on and off
  inputMode(5);   // light sensor is attached to pin 5
}

void loop(){
  lightLevel = read(5);  // read current light level
  
  if (lightLevel == 1){  
      off(0);        // if light level is bright, turn off pin 0     
    } else {
      on(0);         // otherwise turn on pin 0
    }
}