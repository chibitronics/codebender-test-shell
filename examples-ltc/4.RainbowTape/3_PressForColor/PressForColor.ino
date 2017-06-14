// Love to Code

// Press for Color!
//
// Change the colors of pixels by connecting pins to ground.
// Pin 0 changes the color of pixel 0 (on the Chibi Chip board)
// Pin 1 changes the color of pixel 1 (first external pixel)
// and so forth...

// You can set some other color properties by adjusting these constants.
#define RATE 3 // a number larger than 1, less than 20, rate at which the colors change during pin press
#define SATURATION 100  // 0-100, 0 = no color, 100 = vivid colors    
#define INTENSITY 100  // 0-100, 0 = off, 100 = full brightness

#include "Adafruit_NeoPixel.h"

#define pixelCount 5 // maximum 6 in this demo
Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixelCount, LED_BUILTIN_RGB, NEO_GRB + NEO_KHZ800);

typedef struct RgbColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RgbColor;

#define HUE_DEGREE    512 // hue is 0 ... (360*HUE_DEGREE - 1)

// from https://gist.github.com/mity/6034000
void hsv2rgb(int h, int s, int v, RgbColor *c)
{
    uint8_t r, g, b;

    if(s == 0) {
        r = g = b = v;
    } else {
        int i = h / (60*HUE_DEGREE);
        int p = (256*v - s*v) / 256;

        if(i & 1) {
            int q = (256*60*HUE_DEGREE*v - h*s*v + 60*HUE_DEGREE*s*v*i) / (256*60*HUE_DEGREE);
            switch(i) {
                case 1:   r = q; g = v; b = p; break;
                case 3:   r = p; g = q; b = v; break;
                case 5:   r = v; g = p; b = q; break;
            }
        } else {
            int t = (256*60*HUE_DEGREE*v + h*s*v - 60*HUE_DEGREE*s*v*(i+1)) / (256*60*HUE_DEGREE);
            switch(i) {
                case 0:   r = v; g = t; b = p; break;
                case 2:   r = p; g = v; b = t; break;
                case 4:   r = t; g = p; b = v; break;
            }
        }
    }

  c->r = r;
  c->g = g;
  c->b = b;
}

uint32_t rgb_to_hexcolor(RgbColor rgbcolor) {
  return rgbcolor.r << 16 | rgbcolor.g << 8 | rgbcolor.b;
}

int pin_hue[pixelCount];
int pin_dir[pixelCount];

void setup() {
  int i;
  
  for( i = 0; i < pixelCount; i++ ) {
    pinMode(i, INPUT_PULLUP);
    pin_hue[i] = 0;
    pin_dir[i] = RATE;
  }
  
  strip.begin();
  strip.show();
  
  analogReadResolution(10);
}

void loop() {
  RgbColor c;
  int hue;
  unsigned int avg;
  int i, j;
  unsigned int reading;
  
  for( j = 0; j < pixelCount; j++ ) {
    if( digitalRead(j) == LOW ) {
      pin_hue[j] += pin_dir[j];
      if( pin_hue[j] >= HUE_DEGREE ) {
        pin_dir[j] = -pin_dir[j];
        pin_hue[j] = HUE_DEGREE - 1;
      } if( pin_hue[j] < 0 ) {
        pin_dir[j] = -pin_dir[j];
        pin_hue[j] = 0;
      }
    }
  
    hue = map(pin_hue[j], 0, 1023, 0, HUE_DEGREE);
    hue = 360*hue - 1;
  
    hsv2rgb(hue, map(SATURATION, 0, 100, 0, 255), map(INTENSITY, 0, 100, 0, 255), &c);
    
    strip.setPixelColor(j, rgb_to_hexcolor(c));
  }
  
  strip.show();
  delay(10);
}
