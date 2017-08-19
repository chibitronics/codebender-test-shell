// Love to Code
// Volume 1: Fading with While Loop

// In this example we use a while loop to create a smooth 
// fade in and fade out LED effect

int LED = 0;  // initialize LED as pin 0        

void setup() {
  outputMode(LED);  // set LED pin as output
}

void loop() {
  // initialize brightness variable to be our current brightness level
  int brightness = 0;  
  
  // increase brightness while our brightness is less than fully on (100%)
  while(brightness < 100) {
    setLevel(LED, brightness);   //set our LED to the current brightness level
    pause(10);                   // wait for 10 milliseconds for the brightness to show
    brightness = brightness + 1; //increase our brightness level by 1
    // this code repeats until our brightness reaches fully on (100%)
  }
  
  //decrease brightness while our brightness is more than fully off (0%)
  while(brightness > 0) {
    setLevel(LED, brightness);   //set our LED to the current brightness level
    pause(10);                   // wait for 10 milliseconds for the brightness to show
    brightness = brightness - 1; //decrease our brightness level by 1
    // this code repeats until our brightness reaches fully off (0%)
  }
}
