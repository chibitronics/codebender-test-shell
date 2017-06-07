// Love to Code

// Rainbow tape "Fade Color" demo.

// This simple demo enables you to fade a pixel between two colors

#define pixelCount 2 // number of pixels in the chain; doesn't hurt to have less actual pixels in your project
#define dimLevel   1 // a level from 0-5

// select colors codes using the "Color Gadget" below the text editor!
unsigned int colorA = 0x17c7d1;
unsigned int colorB = 0xfd2600;

#include "Adafruit_NeoPixel.h"

typedef struct RgbColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RgbColor;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixelCount, LED_BUILTIN_RGB, NEO_GRB + NEO_KHZ800);

unsigned int dim(unsigned int c, unsigned char level) {
  return( (((c & 0xFF0000) >> level) & 0xFF0000) |
	  (((c & 0x00FF00) >> level) & 0x00FF00) |
	  (((c & 0x0000FF) >> level) & 0x0000FF) );
}

RgbColor ra;
RgbColor rb;
int step = 0;
int rate = 3;
int dim_level;

void setup() {
  int i;
  RgbColor tempcolor;
  
  strip.begin();
  strip.show();

  dim_level = dimLevel;
  
  if( dim_level < 0 )
    dim_level = 0;
  if( dim_level > 5 )
    dim_level = 5;
  
  strip.setPixelColor(0, dim(colorA, dim_level)); 
  strip.show();

  ra.r = (colorA >> 16) & 0xff;
  ra.g = (colorA >> 8) & 0xff;
  ra.b = (colorA >> 0) & 0xff;

  rb.r = (colorB >> 16) & 0xff;
  rb.g = (colorB >> 8) & 0xff;
  rb.b = (colorB >> 0) & 0xff;
}

// blend 8-bit number, alpha is a number 1-100
uint8_t alpha8(uint8_t a, uint8_t b, uint8_t alpha) {
  if( alpha < 1 )
    alpha = 0;
  if( alpha > 100 )
    alpha = 100;

  uint32_t retval = (((uint32_t)a) * alpha) + (((uint32_t)b) * (100 - alpha));
  retval /= 100;
  if( retval > 255 )
    retval = 255;
  return( (uint8_t) retval );
}

void loop() {
  RgbColor blendr;
  uint32_t outc;

  blendr.r = alpha8(ra.r, rb.r, step);
  blendr.g = alpha8(ra.g, rb.g, step);
  blendr.b = alpha8(ra.b, rb.b, step);

  outc = blendr.r << 16 | blendr.g << 8 | blendr.b;
  
  strip.setPixelColor(0, dim(outc, dim_level));
  strip.show();

  step = step + rate;
  if( step <= 0 || step >= 100 ) {
    rate = -rate;
  }
  if( step <= 0 )
    step = 0;
  if( step >= 100 )
    step = 100;
  
  delay(50);
}
