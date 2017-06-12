const int buttonPin = A5;     // the number of the pushbutton pin

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

void setup() {

  pinMode(A0, OUTPUT);      
  pinMode(A1, OUTPUT);     
  pinMode(A2, OUTPUT);     
  pinMode(A3, OUTPUT);     
  pinMode(A4, OUTPUT);     

  pinMode(A5, INPUT_PULLUP);     
}

void loop(){
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  while (buttonState == HIGH) {        
    buttonState = digitalRead(buttonPin);
  }  
 
  fadeOn(A0);
  digitalWrite(A0, HIGH);
  delay (100);
  fadeOff(A0);
  
  fadeOn(A1);
  digitalWrite(A1, HIGH);
  delay (100);
  fadeOff(A1);
  
  fadeOn(A2);
  digitalWrite(A2, HIGH);
  delay (100);
  fadeOff(A2);
  
  fadeOn(A3);
  digitalWrite(A3, HIGH);
  delay (100);
  fadeOff(A3);
  
}

void fadeOff(int ledPin1)
{
  for (int k = 255; k> 0; k=k-5) 
  {
    analogWrite(ledPin1, k);   
    delay(10);
  } 
  analogWrite(ledPin1, 0); 
}

void fadeOn(int ledPin2)
{
    for (int l = 0; l < 255; l=l+5) 
  {
    analogWrite(ledPin2, l);   
    delay(10);
  } 
  analogWrite(ledPin2, 255);
}