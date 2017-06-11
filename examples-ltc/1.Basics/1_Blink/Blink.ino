//  Love to Code

// Blink example!
// Turns on an LED on for one second, then off for one second, repeatedly.
 
// Pick the pin to blink: 0, 1, 2, 3, 4 or 5
int led = 0;

// the setup routine runs once when you press reset:
void setup() {                
  pinMode(led, OUTPUT);      // initialize the digital pin as an output.
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(led, HIGH);   // turn the LED on by applying a high voltage
  delay(1000);               // wait one second (1000 milliseconds)
  digitalWrite(led, LOW);    // turn the LED off by appling a low voltage
  delay(1000);               // wait for a second (1000 milliseconds)
}
