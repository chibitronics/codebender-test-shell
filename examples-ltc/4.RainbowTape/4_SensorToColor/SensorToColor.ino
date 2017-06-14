// Love to Code

// Analog input to color demo
//
// Analog values read on pins 0-4 are translated to color and sent to their repsective pixels
// Pin 5 is a "lock" pin. When high, the analog values can be updated. 
//   When low, the analog values are locked.

// You can set some other color properties by adjusting these constants.
#define SATURATION 90  // 0-100, 0 = no color, 100 = vivid colors    
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

#define NUMFILT 8
unsigned int pin_filter[pixelCount][NUMFILT];
unsigned int pin_index[pixelCount];

void setup() {
  int i, j;
  for( j = 0; j < pixelCount; j++ ) {
    pinMode(j, INPUT); 
    for( i = 0; i < NUMFILT; i++ ) {
      pin_filter[j][i] = 0;
    }
  }
  strip.begin();
  strip.show();
  
  pinMode(5, INPUT_PULLUP);
  analogReadResolution(10);
}

void loop() {
  RgbColor c;
  int hue;
  unsigned int avg;
  int i, j;
  unsigned int reading;
  
  for( j = 0; j < pixelCount; j++ ) {
    if( digitalRead(5) == HIGH ) {
      reading = (unsigned int) analogRead(j);
      pin_filter[j][pin_index[j]] = reading;
      pin_index[j]++;
      pin_index[j] = pin_index[j] % NUMFILT;
    }
    
    avg = 0;
    for( i = 0; i < NUMFILT; i++ ) {
      avg = avg + pin_filter[j][i];
    }
    avg = avg / NUMFILT;
  
    hue = map(avg, 0, 1023, 0, HUE_DEGREE);
    hue = 360*hue - 1;
  
    hsv2rgb(hue, map(SATURATION, 0, 100, 0, 255), map(INTENSITY, 0, 100, 0, 255), &c);
    strip.setPixelColor(j, rgb_to_hexcolor(c));
  }
  
  strip.show();
  delay(10);
}
