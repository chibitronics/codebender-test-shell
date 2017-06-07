// Love to Code

// Positioning text on the ChibiScope 

// By default, the Chibi Scope tries to print whatever it receives
// from the Chibi Chip as fast as it can, line by line. Since the
// Chibi Chip can send data faster than a human can read it, a fast
// loop sending data to the Scope can lead to unexpected results.
// Plus, you have no control over where the text appears on the screen --
// new text always appears on the bottom. 

// If you want precise control of what's printed to the screen,
// You can use special "verbs" to lock the contents of the screen
// and only update contents when commanded. Verbs take the form
// of a hashtag on a line by itself, and they are:

// #LCK -- Lock the contents of the screen
// #SYN -- Synchronize (update) the screen contents from previously sent data
// #RUN -- Unlock the screen and free-run ("normal" mode of operation)

// In this example, we show how to center a single line of text on
// the screen.

int count = 0;  // this is the value we want to display on the screen

void setup(void) {
  Serial.begin(9600);     // setup the TXT port
}

// we put the screen update in "function call" so it's easier to combine
// this code into other projects. Copy and paste the "render" code
// above the loop(), and then call render() inside the loop.
// Don't forget to include the Serial.begin(9600) line inside setup()!
void render() {
  Serial.println("#LCK"); // put the screen into "lock" mode
  
  Serial.print("   Count: "); // print an explanation
  Serial.print( count ); // now print a count value, on the same line
  count = count + 1;

  // spaces are mandatory for a "blank" line. Successive blank lines without spaces in them are ignored.
  // try removing the line below and see what happens!
  Serial.println(" "); // add a blank line to the bottom of the screen
  Serial.println(" "); // add a second blank line on the bottom of the screen
  
  Serial.println("#SYN"); // synchronize the screen contents
  delay(30); // a delay of 30ms is necessary to allow the screen to update, or else the contents are garbled
}

void loop(void) {
  count++;  // count up, so we can see the demo working
  render(); // this jumps to the render code above, keeping the loop() nice and tidy
}
