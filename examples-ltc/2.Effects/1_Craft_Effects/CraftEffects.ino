// Love to Code
// Effects Template
enum effects {  CONSTANT = 0,  FADE = 1,  HEARTBEAT = 2,  TWINKLE = 3 }; // don't delete this line!

////// configuration /////////
////// PICK YOUR EFFECT AND SPEED HERE
////// choose one of CONSTANT, FADE, HEARTBEAT, TWINKLE
////// and a speed from 1 to 25 (higher is faster)
////// See advanced_settings() below for more options
int pin0_effect = CONSTANT;
int pin0_speed = 0;

int pin1_effect = FADE;
int pin1_speed = 1;

int pin2_effect = FADE;
int pin2_speed = 3;

int pin3_effect = HEARTBEAT;
int pin3_speed = 3;

int pin4_effect = HEARTBEAT;
int pin4_speed = 8;

int pin5_effect = TWINKLE;
int pin5_speed = 5;





#include "Arduino.h"
#include "ChibiOS.h"
struct effects_thread_arg {  uint8_t effect;  uint8_t speed;  uint8_t pin; uint8_t brightness; uint8_t randomness; };
static effects_thread_arg pin[6];

void advanced_settings() {
  //////  brightness from 0 to 100 (100 is brighter)
  //////  randomness from 0 to 100 (100 is more random)
  pin[0].brightness = 100;
  pin[0].randomness = 0;
  
  pin[1].brightness = 100;
  pin[1].randomness = 0;
  
  pin[2].brightness = 100;
  pin[2].randomness = 0;
  
  pin[3].brightness = 100;
  pin[3].randomness = 0;
  
  pin[4].brightness = 100;
  pin[4].randomness = 0;
  
  pin[5].brightness = 100;
  pin[5].randomness = 0;
}

/////////////
///////////// implementation below
/////////////
static thread_t *light_threads[6];
  
static int fade_to(int current, int target, int rate, int pin, int pause, int randomness, int brightness) {
  int r;
  while( abs(current - target) > rate ) {
    analogWrite(pin, map(current, 0, 255, 0, brightness));
    current = current + ((target - current) > 0 ? rate : - rate);
    r = random(0, 100);
    if( r < randomness ) {
      if( r % 2 ) {
	if( pause - 1 > 0 )
	  delay( pause - 1 );
      } else {
	delay( pause + 1 );
      }
    } else {
      delay(pause);
    }
  }
  current = target; // handle cases where target and rate aren't multiples of 255
  analogWrite(pin, current);
  return current;
}

static void blink_effect(struct effects_thread_arg *cfg) {
  fade_to( 0, 255, cfg->speed, cfg->pin, 7, cfg->randomness, map(cfg->brightness, 0, 100, 0, 255) );
  fade_to( 255, 0, cfg->speed, cfg->pin, 7, cfg->randomness, map(cfg->brightness, 0, 100, 0, 255) );
}

static void twinkle_effect(struct effects_thread_arg *cfg) {
  int current = 128;
  while(1) {
    current = fade_to(current, random(0, 255), cfg->speed, cfg->pin, 3, cfg->randomness, map(cfg->brightness, 0, 100, 0, 255));
  }
}

static void heartbeat_effect(struct effects_thread_arg *cfg) {
  int current = 0;
  if( cfg->speed > 25 )
    cfg->speed = 25;
  
  current = fade_to(current, 192, 2, cfg->pin, 1, cfg->randomness, map(cfg->brightness, 0, 100, 0, 255));
  current = fade_to(current, 4, 2, cfg->pin, 1, cfg->randomness, map(cfg->brightness, 0, 100, 0, 255));
  delay(40); // fastest rate
  delay( (25 - cfg->speed) * 13 + 1 );
  //delay(180);
  current = fade_to(current, 255, 2, cfg->pin, 1, cfg->randomness, map(cfg->brightness, 0, 100, 0, 255));
  current = fade_to(current, 0, 2, cfg->pin, 1, cfg->randomness, map(cfg->brightness, 0, 100, 0, 255));
  digitalWrite(cfg->pin, 0);
  delay(107); // fastest rate
  delay( (25 - cfg->speed) * 37 + 1 );
  //delay(420);
}

static void effects_thread(void *arg) {
  struct effects_thread_arg *cfg = (struct effects_thread_arg *)arg;
  while (1) {
    switch (cfg->effect) {
      case CONSTANT: exitThread(0); return;
      case FADE: blink_effect(cfg); break;
      case HEARTBEAT: heartbeat_effect(cfg); break;
      case TWINKLE: twinkle_effect(cfg); break;
      default: exitThread(0); return;
    }
  }
}

void setup(void) {
  int i;

  pin[0].effect = pin0_effect;
  pin[0].speed = pin0_speed;

  pin[1].effect = pin1_effect;
  pin[1].speed = pin1_speed;

  pin[2].effect = pin2_effect;
  pin[2].speed = pin2_speed;

  pin[3].effect = pin3_effect;
  pin[3].speed = pin3_speed;
  
  pin[4].effect = pin4_effect;
  pin[4].speed = pin4_speed;

  pin[5].effect = pin5_effect;
  pin[5].speed = pin5_speed;
  
  advanced_settings();
  
  for (i = 0; i < 6; i++) {
    pin[i].pin = i;
    pinMode(pin[i].pin, OUTPUT);
    digitalWrite(pin[i].pin, LOW);
    if (pin[i].effect == CONSTANT) {
      analogWrite(pin[i].pin, map(pin[i].brightness, 0, 100, 0, 255));
      continue;
    }

    if(pin[i].speed > 25)
      pin[i].speed = 25;

    delay( random(1, pin[i].randomness * 5) + 1 );
    light_threads[i] = createThreadFromHeap(32,
                                           20,
                                           effects_thread,
                                           &pin[i]);
  }
}

void loop(void) {
  exitThread(0);
}
