// Love to Code

// Rainbow tape "Simple Color" demo.

// This simple demo enables you to set a static color on RGB pixels.

#define pixelCount 5 // number of pixels in the chain; doesn't hurt to have less actual pixels in your project
#define dimLevel   1 // a level from 0-5

#include "html_colors.h" // add this line to use color names instead of codes
// visit https://github.com/chibitronics/ltc-compiler-layer/blob/master/support/html_colors.h
// for a full list of colors

// select colors codes using the "Color Gadget" below the text editor!
unsigned int pixel_color[pixelCount] = {
  0x17c7d1,
  0x17c7d1,        // hex code for chibi teal (pick from the color gadget)
  0xfd2600,        // hex code for chibi red (pick from the color gadget)
  COLOR_GOLD,      // color by name (if you use html_colors.h)
  COLOR_LIMEGREEN  // color by name (if you use html_colors.h)
};

#include "Adafruit_NeoPixel.h"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixelCount, LED_BUILTIN_RGB, NEO_GRB + NEO_KHZ800);

unsigned int dim(unsigned int c, unsigned char level) {
  return( (((c & 0xFF0000) >> level) & 0xFF0000) |
	  (((c & 0x00FF00) >> level) & 0x00FF00) |
	  (((c & 0x0000FF) >> level) & 0x0000FF) );
}

void setup() {
  int i;
  int dim_level;
  
  strip.begin();
  strip.show();

  dim_level = dimLevel;
  
  if( dim_level < 0 )
    dim_level = 0;
  if( dim_level > 5 )
    dim_level = 5;
  
  for( i = 0; i < pixelCount; i++ ) {
    strip.setPixelColor(i, dim(pixel_color[i], dim_level)); 
  }
  strip.show();
}

void loop() {
  delay(1); // do nothing but idle
}
