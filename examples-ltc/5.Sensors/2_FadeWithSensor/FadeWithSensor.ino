// Love to Code
// Add-on #1: Fade With Sensor

int lightLevel = 0;
  
void setup() {
  outputMode(0);  // pin 0 turns on and off the LED
  inputMode(5);   // pin 5 connects to our light sensor
}

void loop() {
    lightLevel = readLevel(5); // read sensor level
    setLevel(0, lightLevel);   // then set pin 0 to sensor level brightness
    pause(10);                 // tiny pause for the brightness to show
}