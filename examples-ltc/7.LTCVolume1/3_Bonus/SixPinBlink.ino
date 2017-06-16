//Love to Code
//Volume 1: Six Pin Blink

void setup() {
  outputMode(0);
  outputMode(1);
  outputMode(2);
  outputMode(3);
  outputMode(4);
  outputMode(5);
}

void loop() {
  on(0);
  pause(1000);
  off(0);

  on(1);
  pause(1000);
  off(1);

  on(2);
  pause(1000);
  off(2);

  on(3);
  pause(1000);
  off(3);

  on(4);
  pause(1000);
  off(4);

  on(5);
  pause(1000);
  off(5);
}