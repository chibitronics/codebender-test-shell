// Love to Code
// Easy Effects

#include "Arduino.h"
#include "ChibiOS.h"

enum effects {
  NONE = 0,
  BLINK = 1,
  HEARTBEAT = 2,
  TWINKLE = 3,
};

struct effects_thread_arg {
  uint8_t effect;
  uint8_t tempo;
  uint8_t pin;
};

static thread_t *light_threads[6];
static effects_thread_arg port[6];

////// configuration /////////
////// PICK YOUR EFFECT AND TEMPO HERE
////// choose one of NONE, BLINK, HEARTBEAT, TWINKLE
////// and a tempo (generally a higher number makes it go faster)

void configure_effects() {
  port[0].effect = HEARTBEAT;
  port[1].effect = HEARTBEAT;
  port[2].effect = NONE;
  port[3].effect = TWINKLE;
  port[4].effect = TWINKLE;
  port[5].effect = BLINK;

  port[0].tempo = 1;
  port[1].tempo = 12;
  port[2].tempo = 0;
  port[3].tempo = 5;
  port[4].tempo = 2;
  port[5].tempo = 4;
}

/////////////
///////////// implementation below
/////////////
  
static int fade_to(int current, int target, int rate, int pin, int pause) {
  while( abs(current - target) > rate ) {
    analogWrite(pin, current);
    current = current + ((target - current) > 0 ? rate : - rate);
    delay(pause);
  }
  return current;
}

static void blink_effect(struct effects_thread_arg *cfg) {
  fade_to( 0, 255, cfg->tempo, cfg->pin, 7 );
  fade_to( 255, 0, cfg->tempo, cfg->pin, 7 );
}

static void twinkle_effect(struct effects_thread_arg *cfg) {
  int current = 128;
  while(1) {
    current = fade_to(current, random(0, 255), cfg->tempo, cfg->pin, 3);
  }
}

static void heartbeat_effect(struct effects_thread_arg *cfg) {
  int current = 0;
  if( cfg->tempo > 12 )
    cfg->tempo = 12;
  
  current = fade_to(current, 0xc0, 2, cfg->pin, 1);
  current = fade_to(current, 0x4, 2, cfg->pin, 1);
  delay(80); // fastest rate
  delay( (13 - cfg->tempo) * 15 );
  //delay(180);
  current = fade_to(current, 0xff, 2, cfg->pin, 1);
  current = fade_to(current, 0x00, 2, cfg->pin, 1);
  delay(214); // fastest rate
  delay( (13 - cfg->tempo) * 37 );
  //delay(420);
}

static void effects_thread(void *arg) {
  struct effects_thread_arg *cfg = (struct effects_thread_arg *)arg;
  while (1) {
    switch (cfg->effect) {
      case NONE: exitThread(0); return;
      case BLINK: blink_effect(cfg); break;
      case HEARTBEAT: heartbeat_effect(cfg); break;
      case TWINKLE: twinkle_effect(cfg); break;
      default: exitThread(0); return;
    }
  }
}

static thread_t *createThreadFromHeap(size_t size, tprio_t prio,
                                      tfunc_t pf, void *arg) {
  thread_t *thr = (thread_t *)malloc(THD_WORKING_AREA_SIZE(size));
  createThread((void *)thr, THD_WORKING_AREA_SIZE(size), prio, pf, arg);

  /* Mark thr->p_flags as CH_FLAG_MODE_HEAP, so ChibiOS will call free()
   * on the memory after it exits.
   */
  ((uint8_t *)thr)[0x1d] = 1;
  return thr;
}

void setup(void) {
  int i;
  
  configure_effects();

  for (i = 0; i < 6; i++) {
    port[i].pin = i;
    pinMode(port[i].pin, OUTPUT);
    digitalWrite(port[i].pin, LOW);
    if (port[i].effect == NONE)
      continue;
    light_threads[i] = createThreadFromHeap(64,
                                           20,
                                           effects_thread,
                                           &port[i]);
  }
}

void loop(void) {
  exitThread(0);
}
