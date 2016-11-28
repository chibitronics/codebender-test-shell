//  This example is Chibitronics Love to Code tested!
/*
  Multiple tone player
 
 Plays multiple tones on multiple pins in sequence
 
 circuit:
 * 3 8-ohm speaker on pins A3, A2, and A0
 
 created 8 March 2010
 by Tom Igoe 
 based on a snippet from Greg Borenstein
 modified 25 May 2016
 by bunnie

This example code is in the public domain.
 
 http://arduino.cc/en/Tutorial/Tone4
 
 */

void setup() {

}

void loop() {
  // turn off tone function for pin A0:
  noTone(A0);			
  // play a note on pin 6 for 200 ms:
  tone(A3, 440, 200);
  delay(200);

  // turn off tone function for pin A3:
  noTone(A3);
  // play a note on pin 7 for 500 ms:
  tone(A2, 494, 500);
  delay(500);
  
  // turn off tone function for pin A2:
  noTone(A2);  
  // play a note on pin 11 for 500 ms:
  tone(A0, 523, 300);
  delay(300);

}
