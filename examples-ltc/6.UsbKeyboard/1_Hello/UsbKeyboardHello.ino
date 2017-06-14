// Love to Code

// Keyboard emulation demo
//
// 1. Plug the LTC into a USB port on a computer
// 2. Make sure the mouse cursor is focused in an area where it's OK to type
// 3. Load this example
//
// You should see "// Love to Code " typed into your computer once every 3 seconds.

#include "Keyboard.h"

void setup() {
  Keyboard.begin();
}

void loop() {
  Keyboard.print("// Love to Code ");
  delay(3000);
}
