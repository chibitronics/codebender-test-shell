// Love to Code

// Cascading colors

// This example is particularly useful when mapping a few colors
// onto a long strip of rainbow tape or other Neopixel-style (WS2812B) LEDs. 
//
// Color values, specified in pixel_color, are interpolated onto
// a number of pixels that should be larger than the number of
// color values.
//
// The system then animates a cascade effect which lights up the
// colors one at a time down the strip, using a cross-fade effect.

#include "html_colors.h" 
// see https://github.com/chibitronics/ltc-compiler-layer/blob/master/support/html_colors.h

#define PIXELCOUNT 5  // number of pixels total (including the one on the Chibi Chip)
#define RATE  100  // a number from 1-10,000. 100 is a reasonably calm setting.

#define COLORCOUNT 3
unsigned int pixel_color[COLORCOUNT] = {
  COLOR_RED,  
  COLOR_GREEN,
  COLOR_BLUE, 
//  COLOR_YELLOW,  // increase COLORCOUNT and add colors here!
};

#include "Adafruit_NeoPixel.h"
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELCOUNT, LED_BUILTIN_RGB, NEO_GRB + NEO_KHZ800);

typedef struct RgbColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RgbColor;

typedef struct HsvColor {
  int h;
  int s;
  int v;
} HsvColor;

HsvColor hsv_master[COLORCOUNT];

// from https://gist.github.com/mity/6034000
#define HUE_DEGREE    512 // hue is 0 ... (360*HUE_DEGREE - 1)
#define MIN(a,b)      ((a) < (b) ? (a) : (b))
#define MAX(a,b)      ((a) > (b) ? (a) : (b))

#define MIN3(a,b,c)   MIN((a), MIN((b), (c)))
#define MAX3(a,b,c)   MAX((a), MAX((b), (c)))

void rgb2hsv(uint8_t r, uint8_t g, uint8_t b, HsvColor* hsv) {
  int m = MIN3(r, g, b);
  int M = MAX3(r, g, b);
  int delta = M - m;

  if(delta == 0) {
    /* Achromatic case (i.e. grayscale) */
    hsv->h = -1;          /* undefined */
    hsv->s = 0;
  } else {
    int h;

    if(r == M)
      h = ((g-b)*60*HUE_DEGREE) / delta;
    else if(g == M)
      h = ((b-r)*60*HUE_DEGREE) / delta + 120*HUE_DEGREE;
    else /*if(b == M)*/
      h = ((r-g)*60*HUE_DEGREE) / delta + 240*HUE_DEGREE;

    if(h < 0)
      h += 360*HUE_DEGREE;

    hsv->h = h;

    /* The constatnt 8 is tuned to statistically cause as little
     * tolerated mismatches as possible in RGB -> HSV -> RGB conversion.
     * (See the unit test at the bottom of this file.)
     */
    hsv->s = (256*delta-8) / M;
  }
  hsv->v = M;
}

void hsv2rgb(int h, int s, int v, RgbColor *c) {
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

// alpha is 0-100
void interpolate( HsvColor a, HsvColor b, int alpha, HsvColor *c ) {
  c->h = (a.h * alpha + b.h * (100 - alpha)) / 100;
  c->s = (a.s * alpha + b.s * (100 - alpha)) / 100;
  c->v = (a.v * alpha + b.v * (100 - alpha)) / 100;
}

uint8_t alpha8(uint8_t a, uint8_t alpha) {
  uint32_t q;

  if( alpha > 100 )
    alpha = 100;
  
  q = (uint32_t) a * (uint32_t) alpha;

  return (uint8_t) ((q / 100) & 0xFF);
}

RgbColor alpha_rgb(RgbColor a, uint8_t alpha) {
  RgbColor result;

  result.r = alpha8(a.r, alpha);
  result.g = alpha8(a.g, alpha);
  result.b = alpha8(a.b, alpha);

  return result;
}

// map values onto a uniform 16-bit axis, and then re-interpolate
// to the sub-axes of pixel and color count...
#define MAX_AXIS 65535
#define PIXEL_SPACING (MAX_AXIS / PIXELCOUNT)
#define COLOR_SPACING (MAX_AXIS / (COLORCOUNT - 1))

// This computes the brightness cascade.
// x is a number from 0-MAX_AXIS (65535) which indicates where we
// are in the global cascade.
// p is the pixel number we want to compute the local cascade value for
// returns a number from 0-100 which is the brightness mask versus pixel number
unsigned int compute_interpolation( int x, int p ) {
  int retval = 0;
  
  if( x < p * PIXEL_SPACING ) {
    retval = x - (p - 1) * PIXEL_SPACING;
    retval = (retval * 100) / PIXEL_SPACING;
    if( retval < 0 )
      retval = 0;
  } else {
    retval = (p + 1) * PIXEL_SPACING - x;
    retval = (retval * 100) / PIXEL_SPACING;
    if( retval < 0 )
      retval = 0;
  }

  return (unsigned int) retval;
}

// This computes the color of a given pixel
// The color is static: it won't change for a given pixel, but
// the actual color of a pixel must be interpolated from the set
// of colors specified by the user, as there may be many more pixels
// than there are colors
HsvColor compute_color( int p ) {
  HsvColor c;
  HsvColor b;
  
  // figure out which two colors we are in between for a given pixel
  int lower_color = (p * (MAX_AXIS / (PIXELCOUNT - 1))) / COLOR_SPACING;
  HsvColor a = hsv_master[lower_color];
  if( lower_color < (COLORCOUNT-1) )
    b = hsv_master[lower_color + 1];
  else
    b = hsv_master[lower_color];

  //int alpha = ((x - (lower_color * COLOR_SPACING)) * 100) / COLOR_SPACING;
  int x_a = lower_color * COLOR_SPACING;
  int x_b = (lower_color + 1) * COLOR_SPACING;
  int x_p = p * (MAX_AXIS / (PIXELCOUNT - 1));
  
  int alpha = 100 - (x_p - x_a) * 100 / (x_b - x_a);
                     
  interpolate(a, b, alpha, &c);

  return c;
}

void setup() {
  int i;

  // copy RGB colors into HSV space
  for( i = 0; i < COLORCOUNT; i++ ) {
    rgb2hsv( (pixel_color[i] >> 16) & 0xff, (pixel_color[i] >> 8) & 0xff,
	     pixel_color[i] & 0xff, &hsv_master[i] );
  }

  strip.begin();
  strip.show();
}

int x = -PIXEL_SPACING; // state variable that goes from 0 - MAX_AXIS

void loop() {
  int j;
  int alpha;
  HsvColor color_point_hsv;
  RgbColor color_point_rgb;
  RgbColor render_color;

  for( j = 0; j < PIXELCOUNT; j++ ) {
    // figure out the base color of the pixel
    color_point_hsv = compute_color( j );
    hsv2rgb( color_point_hsv.h, color_point_hsv.s, color_point_hsv.v, &color_point_rgb );
    // now figure out the brightness mask given our state variable
    alpha = compute_interpolation( x, j );
    render_color = alpha_rgb( color_point_rgb, alpha );
    // set the color
    strip.setPixelColor(j, rgb_to_hexcolor(render_color));
  }
  x = (x + RATE);
  if( x > MAX_AXIS )
    x = -PIXEL_SPACING;

  strip.show();
  delay(10);
}
