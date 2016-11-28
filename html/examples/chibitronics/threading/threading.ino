#include "Adafruit_NeoPixel.h"
#include "ChibiOS.h"

static THD_WORKING_AREA(blinky_area, 128);
static THD_FUNCTION(do_blinky, arg) {
  (void)arg;

  while (1) {
    digitalWrite(LED_BUILTIN, 1);
    delay(500);
    digitalWrite(LED_BUILTIN, 0);
    delay(1000);
  }
}

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_BUILTIN_RGB,
                                            NEO_GRB + NEO_KHZ800);

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;

  if(WheelPos < 85)
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);

  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }

  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);

  createThread(blinky_area, sizeof(blinky_area), 20, do_blinky, NULL);
  strip.setBrightness(5);
}

void loop(void) {
  static int loopnum = 0;
  int i;

  for(i=0; i<strip.numPixels(); i++)
    strip.setPixelColor(i, Wheel((i+loopnum) & 255));

  strip.show();
  loopnum++;
  delayMicroseconds(10);
}
