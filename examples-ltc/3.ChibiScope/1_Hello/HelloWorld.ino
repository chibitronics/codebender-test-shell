// Love to Code

// A simple "hello world" example using the ChibiScope

// This example sends the words "hello world!" at a very
// high speed to the ChibiScope. As a result, you will see
// some flicker on the bottom line. The cause of this is
// similar to why car wheels appear to spin backwards in movies:
// https://en.wikipedia.org/wiki/Wagon-wheel_effect

// Use the alligator clips to connect TXT, +5V and GND
// between the respective holes on the ChibiChip and ChibiScope

// IMPORTANT NOTE: If you are running the simple "Hello World"
// demo after one of our more advanced text positioning demos,
// you may need to power cycle the Chibi Scope by disconnecting
// +5V and reconnecting it, to exit its freeze-frame animation mode.

void setup() {
  Serial.begin(9600); // ChibiScope "listens" at a fixed speed of 9600 bits per second
}

void loop() {
  Serial.println("Hello world!");
}
