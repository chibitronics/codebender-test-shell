// Love to Code

// This is the default, out-of-box program loaded on the Chibi Chip:
// a cascade of fading LEDs, wiping from left to right

// The demo uses multi-threading and while loops to control every LED
// independently.

#include "ChibiOS.h"

#define HEAP_SIZE 32
static int blink[6] = {0,0,0,0,0,0};

#define FADE_RATE 4

static THD_FUNCTION(do_blinky, arg) {
  int pin = (uint32_t)arg;
  int depth = 0;

  pinMode(pin, OUTPUT);
  digitalWrite(pin, 0);

  while (1) {
    if (blink[pin]) {
      for (depth = 0; depth < 255; depth += FADE_RATE) {
        analogWrite(pin, depth);
        delay(10);
      }
      for (depth = 255; depth > 0; depth -= FADE_RATE) {
        analogWrite(pin, depth);
        delay(10);
      }
      blink[pin] = 0;
    }
    analogWrite(pin, 0);
    delay(1);
  }
}

void setup(void) {
  int i;
  for (i = 0; i < 6; i++)
    createThreadFromHeap(HEAP_SIZE, 20, do_blinky, (void *)i);
}

void loop(void) {

  int j = 0;
  for (j = 0; j < 6; j++) {
    delay(500);
    blink[j] = 1;
  }
  delay(1000); // prevent effects from overlapping
}
