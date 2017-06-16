// Love to Code
// Volume 1: Basic Switch

int pressed = 0;

void setup(){
  outputMode(0);
  inputMode(5);
}

void loop(){
  pressed = read(5);
  
  if (pressed == 1){
      on(0);
      pause(300);
      off(0);
      pause(300);
    } else {
      off(0);
    }
}