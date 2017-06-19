// Love to Code

// A simple "hello world" example using the ChibiScope

// This example sends the words "hello world!" at a very
// high speed to the ChibiScope. As a result, you will see
// some flicker on the bottom line. The cause of this is
// similar to why car wheels appear to spin backwards in movies:
// https://en.wikipedia.org/wiki/Wagon-wheel_effect

// Use the alligator clips to connect TXT, +5V and GND
// between the respective holes on the ChibiChip and ChibiScope

void setup() {
  Serial.begin(9600); // ChibiScope "listens" at a fixed speed of 9600 bits per second

  // the following line is needed to restore the chibiscope mode into
  // its power-on default in case you were previously running a different,
  // more complicated demo that uses the control flow verbs to set text position
  Serial.println("#RUN"); // put the scope into "run" mode, so println's immediately show up
}

void loop() {
  Serial.println("Hello world!");
}
