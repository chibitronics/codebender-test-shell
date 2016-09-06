/*
  Physical programming demo (default for LtC stickers)

  Tap patterns onto buttons A2 or A0 to make the led on A1 or RGB LED flash.
  Pattern can be up to 5 seconds long and is stored in nonvolatile memory (persistent through power outage).

  Pattern is actually stored as an analog value, so if one were to connect
  a variable voltage source to pins A2 or A0, the pattern stored is analog.

  bunnie - June 2 2016
  
 */

#include "Adafruit_NeoPixel.h"
#include "EEPROM.h"

#define LOOP_INTERVAL_MS 25   // sample @ 40 times per second
#define LOOP_LENGTH      200  // 5 second loop total length

uint8_t led_vals[LOOP_LENGTH];
uint8_t rgb_vals[LOOP_LENGTH];

Adafruit_NeoPixel strip = Adafruit_NeoPixel(2, LED_BUILTIN_RGB, NEO_GRB + NEO_KHZ800);

int led = A1;
int ledInput = A2;
int rgbInput = A0;
int recInput = BUTTON_REC;  // special button for physical programming mode

enum operation_modes {
  PLAYBACK = 0,
  RECORD_SETUP,
  RECORDING,
  RECORD_COMMIT,
};

int mode = PLAYBACK;
unsigned long rec_start;
unsigned long play_start;
int lastRecIndex = -1;
int runtimeLength = LOOP_LENGTH;

typedef struct RgbColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RgbColor;

RgbColor hueToRgb(uint8_t hue) {
    RgbColor rgb;
    unsigned char region, remainder, p, q, t;
    uint8_t sat = 235;
    uint8_t val = (255 - hue) / 4; // a little dimmer for all conversions
    
    if( val < 4 ) // get rid of idle flicker
      val = 0;
 
    region = hue / 43;
    remainder = (hue - (region * 43)) * 6;

    p = (val * (255 - sat)) >> 8;
    q = (val * (255 - ((sat * remainder) >> 8))) >> 8;
    t = (val * (255 - ((sat * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = val; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = val; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = val; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = val;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = val;
            break;
        default:
            rgb.r = val; rgb.g = p; rgb.b = q;
            break;
    }
    return rgb;
}


void setup() {
  int i;

  for( i = 0; i < LOOP_LENGTH; i++ ) {
    led_vals[i] = EEPROM.read(i);
    rgb_vals[i] = EEPROM.read(i + LOOP_LENGTH);
  }
  runtimeLength = EEPROM.read(LOOP_LENGTH * 2);
  if(runtimeLength > LOOP_LENGTH)
    runtimeLength = LOOP_LENGTH;
  
  pinMode(led, OUTPUT);
  
  strip.begin();
  strip.show();

  pinMode(ledInput, INPUT);
  pinMode(rgbInput, INPUT);

  pinMode(recInput, INPUT_PULLUP);

  play_start = millis();

  pinMode(A3, OUTPUT);
  digitalWrite(A3, HIGH);  // provide an easy +3V reference for pots on A3

  // this example works fine without the following line of code, but
  // the RGB LED is quite sensitive to noise and activating a hidden pullup
  // feature on the LTC sticker improves the aesthetics by reducing the idle
  // noise of the analog input line.
#ifdef __CHIBITRONICS_LTC__  
  // activate a hidden feature pullup on PTB4 to stabilize color when input is floating
  *((unsigned int *)0x4004A010) |= 0x103; // bonus points for figuring out what this bit of magic does!
#endif

}


void playTimeStep() {
  int index;
  RgbColor rgb;

  index = ((millis() - play_start) / LOOP_INTERVAL_MS) % runtimeLength;

  analogWrite(led, led_vals[index]);
  rgb = hueToRgb(rgb_vals[index]);
  strip.setPixelColor(0, strip.Color(rgb.r, rgb.g, rgb.b));
  strip.setPixelColor(1, strip.Color(rgb.r, rgb.g, rgb.b));
  strip.show();
}

void recordTimeStep() {
  int index;
  int i;
  uint8_t rgbVal;
  uint8_t ledVal;
  RgbColor rgb;

  uint32_t avg;

  index = ((millis() - rec_start) / LOOP_INTERVAL_MS);
  if( index == lastRecIndex )  // already  sampled this interval's data, don't do anything
    return;

  if( index >= LOOP_LENGTH ) { // sampled through end of loop, don't do anything
    runtimeLength = LOOP_LENGTH;
    return;
  }
  
  runtimeLength = index;

  // make sure we've advanced exactly one time step; if not, backfill gaps
  if( (index - lastRecIndex) != 1 ) {
    if( (lastRecIndex >= 0) && (lastRecIndex < LOOP_LENGTH ) ) {
      rgbVal = rgb_vals[lastRecIndex];
      ledVal = led_vals[lastRecIndex];
    } else {
      // this is the case that our first iteration into here took longer than LOOP_INTERVAL_MS to arrive...
      rgbVal = 255; // just a default value 
      ledVal = 255;
    }
    // just copy the last known good value forward; maybe more intelligent to interpolate if this was really a problem
    // you encounter this mostly when you insert a lot of serial debug infos in the loop
    for( i = lastRecIndex + 1; i < index; i++ ) {
      rgb_vals[i] = rgbVal;
      led_vals[i] = ledVal;
    }
  }

  // put a little more averaging on the RGB input because it's very sensitive to noise
  avg = 0;
  for( i = 0; i < 8; i++ ) {
    avg += analogRead(rgbInput);
  }
  avg /= 8;
  // now grab the new index value
  rgbVal = (uint8_t) (avg / 16);  // 4096 is max analog value (hardware constraint)
  ledVal = (uint8_t) (analogRead(ledInput) / 16);
  
  rgb_vals[index] = rgbVal;
  led_vals[index] = ledVal;

  lastRecIndex = index;
  
  // reflect this out the status LEDs
  analogWrite(led, led_vals[index]);
  rgb = hueToRgb(rgb_vals[index]);
  strip.setPixelColor(0, strip.Color(rgb.r, rgb.g, rgb.b));
  strip.setPixelColor(1, strip.Color(rgb.r, rgb.g, rgb.b));
  strip.show();
}

void flushData() {
  int i;
  // the hardware does better with sequential writes than interleaved writes
  // we also don't want to continuously write to EEPROM because on the Chibi LtC this is acutally FLASH
  // which can wear out after 10k write cycles
  for( i = 0; i < LOOP_LENGTH; i++ ) {
    EEPROM.write(i, led_vals[i]);
  }
  for( i = 0; i < LOOP_LENGTH; i++ ) {
    EEPROM.write(i + LOOP_LENGTH, rgb_vals[i]);
  }
  EEPROM.write(LOOP_LENGTH * 2, runtimeLength);
  
}

void loop() {

  switch(mode) {
  case PLAYBACK:
    playTimeStep();
    if( digitalRead(BUTTON_REC) == LOW )
      mode = RECORD_SETUP;
    break;
  case RECORD_SETUP:
    rec_start = millis();
    lastRecIndex = -1;
    mode = RECORDING;
    break;
  case RECORDING:
    recordTimeStep();
    if( digitalRead(BUTTON_REC) == HIGH )
      mode = RECORD_COMMIT;
    break;
  case RECORD_COMMIT:
    flushData();
    play_start = millis();
    mode = PLAYBACK;
    break;
  default:
    mode = PLAYBACK;
  }
}
