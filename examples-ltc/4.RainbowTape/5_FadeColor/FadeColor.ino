// Love to Code

// Rainbow tape "Fade Color" demo.

// This simple demo enables you to fade a pixel between two colors

// select colors codes using the "Color Gadget" below the text editor,
// or use any color from https://github.com/chibitronics/ltc-compiler-layer/blob/master/support/html_colors.h
#include "html_colors.h"
int pix0_colorA = 0x17c7d1;      // can specify color as hex code
int pix0_colorB = COLOR_DARKRED; // or specify color as html-standard name
int pix0_speed = 3;              // a number 0-100
int pix0_brightness = 50;        // a number 0-100

int pix1_colorA = COLOR_GREEN;
int pix1_colorB = COLOR_RED;
int pix1_speed = 5;
int pix1_brightness = 20;

int pix2_colorA = COLOR_BLUE;
int pix2_colorB = COLOR_HOTPINK;
int pix2_speed = 4;
int pix2_brightness = 100;

int pix3_colorA = COLOR_TEAL;
int pix3_colorB = COLOR_FUCHSIA;
int pix3_speed = 3;
int pix3_brightness = 80;

int pix4_colorA = COLOR_ROYALBLUE;
int pix4_colorB = COLOR_SEASHELL;
int pix4_speed = 2;
int pix4_brightness = 15;

#include "Adafruit_NeoPixel.h"

#define pixelCount 5 // number of pixels in the chain; doesn't hurt to have less actual pixels in your project
Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixelCount, LED_BUILTIN_RGB, NEO_GRB + NEO_KHZ800);

typedef struct RgbColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RgbColor;

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
  RgbColor colorA;
  RgbColor colorB;
  int speed;
  uint8_t brightness;
  int state;
};
static pix_config config[pixelCount]; 

RgbColor rgb_a;
RgbColor rgb_b;
int step = 0;
int rate = 3;

void setup() {
  int i;

  strip.begin();
  strip.show();

  config[0].colorA = hexcolor_to_rgb(pix0_colorA);
  config[0].colorB = hexcolor_to_rgb(pix0_colorB);
  config[0].speed = pix0_speed;
  config[0].brightness = pix0_brightness;

  config[1].colorA = hexcolor_to_rgb(pix1_colorA);
  config[1].colorB = hexcolor_to_rgb(pix1_colorB);
  config[1].speed = pix1_speed;
  config[1].brightness = pix1_brightness;
  
  config[2].colorA = hexcolor_to_rgb(pix2_colorA);
  config[2].colorB = hexcolor_to_rgb(pix2_colorB);
  config[2].speed = pix2_speed;
  config[2].brightness = pix2_brightness;

  config[3].colorA = hexcolor_to_rgb(pix3_colorA);
  config[3].colorB = hexcolor_to_rgb(pix3_colorB);
  config[3].speed = pix3_speed;
  config[3].brightness = pix3_brightness;

  config[4].colorA = hexcolor_to_rgb(pix4_colorA);
  config[4].colorB = hexcolor_to_rgb(pix4_colorB);
  config[4].speed = pix4_speed;
  config[4].brightness = pix4_brightness;

  for( i = 0; i < pixelCount; i++ ) {
    config[i].state = 0;
    strip.setPixelColor(i, rgb_to_hexcolor(alpha_rgb(config[i].colorA, config[i].brightness)));
  }
  
  strip.show();
}

void loop() {
  RgbColor blended;
  uint8_t fadevalue;
  int i;

  for( i = 0; i < pixelCount; i++ ) {
    blended = blend_rgb( config[i].colorA, config[i].colorB, config[i].state );

    if( config[i].state < 50 ) {
      fadevalue = 100 - 2 * config[i].state;
    } else {
      fadevalue = (config[i].state - 50) * 2;
    }
    blended = alpha_rgb( blended, fadevalue );
    blended = alpha_rgb( blended, config[i].brightness );

    strip.setPixelColor(i, rgb_to_hexcolor(blended));
    
    config[i].state = config[i].state + config[i].speed;
    if( config[i].state <= 0 || config[i].state >= 100 ) {
      config[i].speed = -config[i].speed;
    }
    if( config[i].state <= 0 )
      config[i].state = 0;
    if( config[i].state >= 100 )
      config[i].state = 100;
  }

  strip.show();
  
  delay(50);
}
