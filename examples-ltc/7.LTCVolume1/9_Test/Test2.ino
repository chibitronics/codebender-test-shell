// Love to Code

// A fade demo using more of the API calls

void setup() {
  outputMode(0);
}

int pin = 5;
int fadeAmount = 5;
int brightness = 0;

void loop() {
  setLevel(pin, brightness);

  brightness = brightness + fadeAmount;

  if (brightness <= 0 || brightness >= 100) {
    fadeAmount = -fadeAmount ; 
  }     

  pause(30);                            
}

