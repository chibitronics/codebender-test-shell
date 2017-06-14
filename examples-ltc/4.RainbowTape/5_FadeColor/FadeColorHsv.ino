// Love to Code

// Rainbow tape "Fade Color" demo in HSV space.

// In this variant, we fade using HSV space, instead of alpha blending
// We also add randomness and depth, so we can use this demo to create
// simple flame effects, as demonstrated on the default pix0 configuration

// select colors codes using the "Rainbow Gadget".
// or use any color from https://github.com/chibitronics/ltc-compiler-layer/blob/master/support/html_colors.h
#include "html_colors.h"
int pix0_colorA = COLOR_ORANGERED;  // can specify color as hex code
int pix0_colorB = COLOR_RED;   // or specify color as html-standard name
int pix0_speed = 3;             // a number 0-100, how fast the transitions happen
int pix0_brightness = 80;       // a number 0-100, how bright the overall effect is
int pix0_randomness = 20;       // a number 0-100, how random the timing is between color transitions
int pix0_depth = 20;            // a number 0-100, how dark to get between transitions

int pix1_colorA = COLOR_GREEN;
int pix1_colorB = COLOR_RED;
int pix1_speed = 5;
int pix1_brightness = 20;
int pix1_randomness = 0;
int pix1_depth = 100;

int pix2_colorA = COLOR_BLUE;
int pix2_colorB = COLOR_HOTPINK;
int pix2_speed = 4;
int pix2_brightness = 100;
int pix2_randomness = 0;
int pix2_depth = 100;

int pix3_colorA = COLOR_TEAL;
int pix3_colorB = COLOR_FUCHSIA;
int pix3_speed = 3;
int pix3_brightness = 80;
int pix3_randomness = 0;
int pix3_depth = 100;

int pix4_colorA = COLOR_RED;
int pix4_colorB = COLOR_BLUE;
int pix4_speed = 2;
int pix4_brightness = 15;
int pix4_randomness = 0;
int pix4_depth = 100;

#include "Adafruit_NeoPixel.h"

#define pixelCount 5 // number of pixels in the chain; doesn't hurt to have less actual pixels in your project
Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixelCount, LED_BUILTIN_RGB, NEO_GRB + NEO_KHZ800);

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

// blend 8-bit number, alpha is a number 1-100
uint8_t blend8(uint8_t a, uint8_t b, uint8_t alpha) {
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

RgbColor blend_rgb(RgbColor a, RgbColor b, uint8_t alpha) {
  RgbColor result;

  result.r = blend8(a.r, b.r, alpha);
  result.g = blend8(a.g, b.g, alpha);
  result.b = blend8(a.b, b.b, alpha);

  return result;
}

// alpha is 0-100
void blend_hsv( HsvColor a, HsvColor b, int alpha, HsvColor *c ) {
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

RgbColor hexcolor_to_rgb(uint32_t hexcolor) {
  RgbColor result;

  result.r = (hexcolor >> 16) & 0xff;
  result.g = (hexcolor >> 8) & 0xff;
  result.b = (hexcolor >> 0) & 0xff;

  return result;
}

uint32_t rgb_to_hexcolor(RgbColor rgbcolor) {
  return rgbcolor.r << 16 | rgbcolor.g << 8 | rgbcolor.b;
}

struct pix_config {
  HsvColor colorA;
  HsvColor colorB;
  int speed;
  uint8_t brightness;
  int state;
  int randomness;
  int depth;
};
static pix_config config[pixelCount]; 

RgbColor rgb_a;
RgbColor rgb_b;
int step = 0;
int rate = 3;

void setup() {
  int i;
  RgbColor c;

  strip.begin();
  strip.show();

  rgb2hsv((pix0_colorA >> 16) & 0xff, (pix0_colorA >> 8 & 0xff), pix0_colorA & 0xff,
	  &config[0].colorA);
  rgb2hsv((pix0_colorB >> 16) & 0xff, (pix0_colorB >> 8 & 0xff), pix0_colorB & 0xff,
	  &config[0].colorB);
  config[0].speed = pix0_speed;
  config[0].brightness = pix0_brightness;
  config[0].randomness = pix0_randomness;
  config[0].depth = pix0_depth;

  rgb2hsv((pix1_colorA >> 16) & 0xff, (pix1_colorA >> 8 & 0xff), pix1_colorA & 0xff,
	  &config[1].colorA);
  rgb2hsv((pix1_colorB >> 16) & 0xff, (pix1_colorB >> 8 & 0xff), pix1_colorB & 0xff,
	  &config[1].colorB);
  config[1].speed = pix1_speed;
  config[1].brightness = pix1_brightness;
  config[1].randomness = pix1_randomness;
  config[1].depth = pix1_depth;
  
  rgb2hsv((pix2_colorA >> 16) & 0xff, (pix2_colorA >> 8 & 0xff), pix2_colorA & 0xff,
	  &config[2].colorA);
  rgb2hsv((pix2_colorB >> 26) & 0xff, (pix2_colorB >> 8 & 0xff), pix2_colorB & 0xff,
	  &config[2].colorB);
  config[2].speed = pix2_speed;
  config[2].brightness = pix2_brightness;
  config[2].randomness = pix2_randomness;
  config[2].depth = pix2_depth;

  rgb2hsv((pix3_colorA >> 16) & 0xff, (pix3_colorA >> 8 & 0xff), pix3_colorA & 0xff,
	  &config[3].colorA);
  rgb2hsv((pix3_colorB >> 26) & 0xff, (pix3_colorB >> 8 & 0xff), pix3_colorB & 0xff,
	  &config[3].colorB);
  config[3].speed = pix3_speed;
  config[3].brightness = pix3_brightness;
  config[3].randomness = pix3_randomness;
  config[3].depth = pix3_depth;

  rgb2hsv((pix4_colorA >> 16) & 0xff, (pix4_colorA >> 8 & 0xff), pix4_colorA & 0xff,
	  &config[4].colorA);
  rgb2hsv((pix4_colorB >> 26) & 0xff, (pix4_colorB >> 8 & 0xff), pix4_colorB & 0xff,
	  &config[4].colorB);
  config[4].speed = pix4_speed;
  config[4].brightness = pix4_brightness;
  config[4].randomness = pix4_randomness;
  config[4].depth = pix4_depth;

  for( i = 0; i < pixelCount; i++ ) {
    config[i].state = 0;
    hsv2rgb(config[i].colorA.h, config[i].colorA.s, config[i].colorA.v, &c);
    strip.setPixelColor(i, rgb_to_hexcolor(alpha_rgb(c, config[i].brightness)));
  }
  
  strip.show();
}

void loop() {
  HsvColor blended;
  RgbColor blended_rgb;
  uint8_t fadevalue;
  int i;

  for( i = 0; i < pixelCount; i++ ) {
    blend_hsv( config[i].colorA, config[i].colorB, config[i].state, &blended );

    if( config[i].state < 50 ) {
      fadevalue = 100 - 2 * config[i].state;
    } else {
      fadevalue = (config[i].state - 50) * 2;
    }
    hsv2rgb( blended.h, blended.s, blended.v, &blended_rgb );
    blended_rgb = alpha_rgb( blended_rgb, fadevalue );
    blended_rgb = alpha_rgb( blended_rgb, config[i].brightness );

    strip.setPixelColor(i, rgb_to_hexcolor(blended_rgb));
    
    config[i].state = config[i].state + config[i].speed;
    if( config[i].speed > 0 )
      config[i].state = config[i].state + random(0, config[i].randomness);
    else
      config[i].state = config[i].state - random(0, config[i].randomness);
    if( config[i].state <= (100 - config[i].depth) || config[i].state >= 100 ) {
      config[i].speed = -config[i].speed;
    }
    if( config[i].state <= (100 - config[i].depth) )
      config[i].state = (100 - config[i].depth);
    if( config[i].state >= 100 )
      config[i].state = 100;
  }

  strip.show();
  
  delay(50);
}
