// Love to Code
// Add-on #1: Basic Light Sensor

int lightLevel = 0;
  
void setup() {
  outputMode(0);  // pin 0 turns on and off the LED
  inputMode(5);   // pin 5 connects to our light sensor
}

void loop(){
    lightLevel = read(5); // read light sensor

    if(lightLevel == 1) {
      off(0);   // if bright, turn OFF pin 0
    } else {
      on(0);    // otherwise, turn ON pin 0
    } 
}  