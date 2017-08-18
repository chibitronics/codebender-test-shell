/* Chipdisk demo -- generate 8-bit PWM procedurally generated music
 *
 * Hook up a speaker or headphones between pins 0 and 1 -- this uses a bridge drive (push/pull) topology
 * 
 * Touch pin 4 for "Previous Song"
 * Touch pin 5 for "Next Song"
 */

#include "Arduino.h"
#include "ChibiOS.h"
#include "kl02.h"
#include "memio.h"

// The system is running off of a 32.768 kHz crystal going through
// a 1464x FLL multiplier, giving a system frequency of
// 47.972352 MHz.

// The number of ticks that the sound system has gone through.
// Overflows after about three days, at 14 kHz.
volatile uint32_t global_tick_counter;

// The next sample to be played, nominally between -128 and 127
static volatile int32_t next_sample;

// Nonzero if a sample has been queued, zero if the sample buffer is empty.
static volatile uint8_t sample_queued;

struct song
{
  uint8_t (*generator)(uint32_t t);
  uint8_t delay_loops;
};

// mu6k http://www.youtube.com/watch?v=tCRPUv8V22o 32.0 kHz
static uint8_t mu6k_generator(uint32_t t)
{
  uint32_t y = 0;
  uint32_t x = 0;
  return (((int)(3e3 / (y = t & 16383)) & 1) * 35) +
         (x = t * ("6689"[t >> 16 & 3] & 15) / 24 & 127) * y / 4e4 +
         (((t >> 8 ^ t >> 10) | t >> 14 | x) & 63);
}

// kb 2011-10-04 http://pouet.net/topic.php?which=8357&page=8 44kHz
static uint8_t kb1_generator(uint32_t t)
{
  return ((t / 2 * (15 & (0x234568a0 >> (t >> 8 & 28)))) | t / 2 >> (t >> 11) ^ t >> 12) + (t / 16 & t & 24);
}

// stephth 2011-10-03 http://news.ycombinator.com/item?id=3063359
static uint8_t stephth_generator(uint32_t t)
{
  return (t * 9 & t >> 4 | t * 5 & t >> 7 | t * 3 & t / 1024) - 1;
}

// ryg 2011-10-10 http://www.youtube.com/watch?v=tCRPUv8V22o 44.1 kHz
static uint8_t ryg_generator(uint32_t t)
{
  return ((t * ("36364689"[t >> 13 & 7] & 15)) / 12 & 128) + (((((t >> 12) ^ (t >> 12) - 2) % 11 * t) / 4 | t >> 13) & 127);
}

// xpansive 2011-09-29 http://pouet.net/topic.php?which=8357&page=2 "Lost in Space"
static uint8_t xpansive_generator(uint32_t t)
{
  return ((t * (t >> 8 | t >> 9) & 46 & t >> 8)) ^ (t & t >> 13 | t >> 6);
}

// rez 2011-10-05 http://pouet.net/topic.php?which=8357&page=11 js-only optimized by ryg
static uint8_t rez_generator(uint32_t t)
{
  return t * (1 + "4451"[t >> 13 & 3] / 10) & t >> 9 + ((int)(t * 0.003) & 3);
}

// visy 2011-10-06 http://pouet.net/topic.php?which=8357&page=13
static uint8_t visy_generator(uint32_t t)
{
  return (t % 25 - (t >> 2 | t * 15 | t % 227) - t >> 3) | ((t >> 5) & (t << 5) * 1663 | (t >> 3) % 1544) / (t % 17 | t % 2048);
}

// bear @ celephais
static uint8_t bear_gen(uint32_t t)
{
  return t >> 6 ^ t & 37 | t + (t ^ t >> 11) - t * ((t % 24 ? 2 : 6) & t >> 11) ^ t << 1 & (t & 598 ? t >> 4 : t >> 10);
}

static struct song songs[] = {
    {
        .generator = mu6k_generator,
        .delay_loops = 3,
    },
    {
        .generator = kb1_generator,
        .delay_loops = 1,
    },
    {
        .generator = stephth_generator,
        .delay_loops = 14,
    },
    {
        .generator = ryg_generator,
        .delay_loops = 1,
    },
    {
        .generator = xpansive_generator,
        .delay_loops = 13,
    },
    {
        .generator = rez_generator,
        .delay_loops = 13,
    },
    {
        .generator = visy_generator,
        .delay_loops = 13,
    },
    {
        .generator = bear_gen,
        .delay_loops = 13,
    }};
static struct song *current_song = &songs[0];
static uint32_t current_song_idx = 0;

static int pwm0_stable_timer(void)
{
  static int loops = 0;

  /* Reset the timer IRQ, to allow us to fire again next time */
  writel(TPM0_STATUS_CH1F | TPM0_STATUS_CH0F | TPM0_STATUS_TOF, TPM0_STATUS);

  if (loops++ > current_song->delay_loops)
  {
    int32_t scaled_sample = next_sample + 130;
    if (scaled_sample > 255)
      scaled_sample = 255;
    if (scaled_sample <= 0)
      scaled_sample = 1;
    writel(scaled_sample, TPM0_C1V);
    writel(scaled_sample, TPM0_C0V);

    loops = 0;
    sample_queued = 0;
    global_tick_counter++;
  }

  return 0;
}

static void prepare_pwm()
{
  // Write dummy values out, to configure PWM mux
  pinMode(0, OUTPUT);
  analogWrite(0, 63);
  pinMode(1, OUTPUT);
  analogWrite(1, 63);

  // Disable TPM0, allowing us to configure it
  writel(0, TPM0_SC);

  // Also disable both channels, which are running from the
  // calls to analogWrite() above
  writel(0, TPM0_C0SC);
  writel(0, TPM0_C1SC);

  // Configure the TPM to use the MCGFLLCLK (~32 MHz?)
  writel(readl(SIM_SOPT2) | (1 << 24), SIM_SOPT2);

  // We've picked pin 0, which is on TPM0_CH1
  writel(255, TPM0_MOD);
  writel(0, TPM0_CNT);

  writel(TPM0_C0SC_MSB | TPM0_C0SC_ELSB, TPM0_C0SC);
  writel(TPM0_C1SC_MSB | TPM0_C1SC_ELSA, TPM0_C1SC);

  writel(100, TPM0_C1V);
  writel(100, TPM0_C0V);
  writel(TPM0_SC_TOF | TPM0_SC_TOIE | TPM0_SC_CMOD(1) | TPM0_SC_PS(0), TPM0_SC); // Enable TPM0

  /* Enable the IRQ in the system-wide interrupt table */
  attachFastInterrupt(PWM0_IRQ, pwm0_stable_timer);
}

static void touch_thread(void *ignored)
{
  uint32_t start_time;
  const uint32_t next_pin = 5, prev_pin = 4;
  static uint8_t last_state;

  while (1)
  {
    // Set pin 1 high.
    pinMode(next_pin, OUTPUT);
    pinMode(prev_pin, OUTPUT);
    digitalWrite(next_pin, HIGH);
    digitalWrite(prev_pin, HIGH);
    // Wait a moment for it to charge.
    delay(2);

    // Set the pin back to an input and wait for it to change.
    start_time = micros();
    pinMode(next_pin, INPUT);
    pinMode(prev_pin, INPUT);
    uint32_t end_time_next = 0;
    uint32_t end_time_prev = 0;
    uint8_t next_val, prev_val;
    do
    {
      next_val = digitalRead(next_pin);
      prev_val = digitalRead(prev_pin);
      if (end_time_next == 0 && !next_val)
      {
        end_time_next = micros();
      }
      if (end_time_prev == 0 && !prev_val)
        end_time_prev = micros();
    } while (next_val || prev_val);

    if ((end_time_next - start_time) > 100)
    {
      if (last_state != 1)
      {
        current_song_idx++;
        if (current_song_idx >= (sizeof(songs) / sizeof(*songs)))
          current_song_idx = 0;
        current_song = &songs[current_song_idx];
        global_tick_counter = 0;
      }
      last_state = 1;
    }
    else if ((end_time_prev - start_time) > 100)
    {
      if (last_state != 1)
      {
        if (current_song_idx == 0)
          current_song_idx = (sizeof(songs) / sizeof(*songs)) - 1;
        else
          current_song_idx--;

        current_song = &songs[current_song_idx];
        global_tick_counter = 0;
      }
      last_state = 1;
    }
    else
      last_state = 0;

    delay(50);
  }
}

void setup(void)
{
  prepare_pwm();
  enableInterrupt(PWM0_IRQ);
  createThreadFromHeap(64, 120, touch_thread, NULL);
}

void loop(void)
{

  // If a sample is still in the buffer, don't do anything.
  if (sample_queued)
    return;

  next_sample = (current_song->generator(global_tick_counter) & 0xff) - 128;
  sample_queued = 1;
}
