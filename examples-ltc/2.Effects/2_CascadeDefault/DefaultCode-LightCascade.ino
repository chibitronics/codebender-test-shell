// Love to Code

// This is the default, out-of-box program loaded on the Chibi Chip:
// a cascade of fading LEDs, wiping from left to right

// The demo uses multi-threading to control every LED independently.

#include "ChibiOS.h"

int blink[6] = {0,0,0,0,0,0};

#define FADE_RATE 4

static THD_WORKING_AREA(blinky_area, 64);
static THD_FUNCTION(do_blinky, arg) {
  (void)arg;
  int depth = 0;

  while (1) {
    if (blink[0]) {
      for (depth = 0; depth < 255; depth += FADE_RATE) {
        analogWrite(A0, depth);
        delay(10);
      }
      for (depth = 255; depth > 0; depth -= FADE_RATE) {
        analogWrite(A0, depth);
        delay(10);
      }
      blink[0] = 0;
    }
    analogWrite(A0, 0);
    delay(1);
  }
}

static THD_WORKING_AREA(blinky_area1, 64);
static THD_FUNCTION(do_blinky1, arg) {
  (void)arg;
  int depth = 0;

  while (1) {
    if( blink[1] ) {
      for( depth = 0; depth < 255; depth += FADE_RATE ) {
        analogWrite(A1, depth);
        delay(10);
      }
      for( depth = 255; depth > 0; depth -= FADE_RATE ) {
        analogWrite(A1, depth);
        delay(10);
      }
      blink[1] = 0;
    }
    analogWrite(A1, 0);
    delay(1);
  }
}

static THD_WORKING_AREA(blinky_area2, 64);
static THD_FUNCTION(do_blinky2, arg) {
  (void)arg;
  int depth = 0;

  while (1) {
    if( blink[2] ) {
      for( depth = 0; depth < 255; depth += FADE_RATE ) {
        analogWrite(A2, depth);
        delay(10);
      }
      for( depth = 255; depth > 0; depth -= FADE_RATE ) {
        analogWrite(A2, depth);
        delay(10);
      }
      blink[2] = 0;
    }
    analogWrite(A2, 0);
    delay(1);
  }
}

static THD_WORKING_AREA(blinky_area3, 64);
static THD_FUNCTION(do_blinky3, arg) {
  (void)arg;
  int depth = 0;

  while (1) {
    if( blink[3] ) {
      for( depth = 0; depth < 255; depth += FADE_RATE ) {
        analogWrite(A3, depth);
        delay(10);
      }
      for( depth = 255; depth > 0; depth -= FADE_RATE ) {
        analogWrite(A3, depth);
        delay(10);
      }
      blink[3] = 0;
    }
    analogWrite(A3, 0);
    delay(1);
  }
}

static THD_WORKING_AREA(blinky_area4, 64);
static THD_FUNCTION(do_blinky4, arg) {
  (void)arg;
  int depth = 0;

  while (1) {
    if( blink[4] ) {
      for( depth = 0; depth < 255; depth += FADE_RATE ) {
        analogWrite(A4, depth);
        delay(10);
      }
      for( depth = 255; depth > 0; depth -= FADE_RATE ) {
        analogWrite(A4, depth);
        delay(10);
      }
      blink[4] = 0;
    }
    analogWrite(A4, 0);
    delay(1);
  }
}

static THD_WORKING_AREA(blinky_area5, 64);
static THD_FUNCTION(do_blinky5, arg) {
  (void)arg;
  int depth = 0;

  while (1) {
    if( blink[5] ) {
      for (depth = 0; depth < 255; depth += FADE_RATE) {
        analogWrite(A5, depth);
        delay(10);
      }
      for (depth = 255; depth > 0; depth -= FADE_RATE) {
        analogWrite(A5, depth);
        delay(10);
      }
      blink[5] = 0;
    }
    analogWrite(A5, 0);
    delay(1);
  }
}


void setup(void) {
  pinMode(A0, OUTPUT);
  digitalWrite(A0, 0);

  pinMode(A1, OUTPUT);
  digitalWrite(A1, 0);

  pinMode(A2, OUTPUT);
  digitalWrite(A2, 0);

  pinMode(A3, OUTPUT);
  digitalWrite(A3, 0);

  pinMode(D0, OUTPUT);
  digitalWrite(D0, 0);

  pinMode(D1, OUTPUT);
  digitalWrite(D1, 0);

  createThread(blinky_area, sizeof(blinky_area), 20, do_blinky, NULL);
  createThread(blinky_area1, sizeof(blinky_area1), 20, do_blinky1, NULL);
  createThread(blinky_area2, sizeof(blinky_area2), 20, do_blinky2, NULL);
  createThread(blinky_area3, sizeof(blinky_area3), 20, do_blinky3, NULL);
  createThread(blinky_area4, sizeof(blinky_area4), 20, do_blinky4, NULL);
  createThread(blinky_area5, sizeof(blinky_area5), 20, do_blinky5, NULL);
}

void loop(void) {

  int j = 0;
  for (j = 0; j < 6; j++) {
    delay(500);
    blink[j] = 1;
  }
  delay(1000); // added to keep patterns from overlapping
}
