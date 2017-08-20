// Love to Code
// Volume 1: Set Level

int LED = 0;        // initialize LED as pin 0        

void setup() {
  outputMode(LED);  // set LED pin as output
}

// Use repeated setLevel(pin, percent) statements
// to increase brightness level to 100 (fully on)
// and then back down to 0 (fully off)
void loop() {
  setLevel(LED, 25);
  pause(500);
  setLevel(LED, 50);
  pause(500);
  setLevel(LED, 75);
  pause(500);
  setLevel(LED, 100);
  pause(500);
  setLevel(LED, 75);
  pause(500);
  setLevel(LED, 50);
  pause(500);
  setLevel(LED, 25);
  pause(500);
  setLevel(LED, 0);
  pause(500);
}
