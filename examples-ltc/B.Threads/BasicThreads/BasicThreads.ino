// Love to Code

// This is a simple demonstration of how to create two threads to control
// two different LEDs.

// Each thread can run the same code in this sketch at the same
// time. You have to be careful when running multiple threads because
// if they talk to the same pin or variable at the same time, the
// results can be confusing unless you are very very careful about
// coordinating who talks when.
//
// To avoid this confusion, in all our examples we use a "one thread,
// one pin" rule. If you break this rule, expect some confusing results!
//
// Threads share global variables and "heap" (if you don't know what heap is,
// don't worry about it). However, each thread has its own "stack": variables
// local to a function call are private to the thread. We only give a little
// bit of memory to each thread, but if you start to use arrays inside your
// procedures, you'll want to increase the amount of memory available to the
// thread to keep the Chibi Chip from crashing.

#include "ChibiOS.h" // we need to include our special functions to enable threading

// Each thread gets a little bit of local memory. If you start using things
// like arrays in your thread functions, you might want to increase this.
// Memory is precious on the LTC, you only have a few hundred bytes total to
// share between all the threads!
#define THREAD_MEMORY 64
// higher numbers will get run first if there's a scheduling conflict
// The highest number is 127
#define THREAD_PRIORITY 20

// You can make more threads if you want, to control more pins!
#define NUMBER_OF_THREADS 2

// Each thread runs the same piece of code. If you hard-coded the pin
// number into the thread code, then all the threads would talk to the
// same pin!  The following data structure allows you to configure a
// thread's pin and effect speed
struct fade_thread_config {
  uint8_t speed;
  uint8_t pin;
};
static fade_thread_config thread_config[NUMBER_OF_THREADS];

static thread_t *fade_threads[NUMBER_OF_THREADS];

// this magic function creates a Thread from heap
static thread_t *runThread(tfunc_t pf, void *arg) {
  thread_t *thr = (thread_t *)malloc(THD_WORKING_AREA_SIZE(THREAD_MEMORY));
  createThread((void *)thr, THD_WORKING_AREA_SIZE(THREAD_MEMORY), THREAD_PRIORITY, pf, arg);

  /* Mark thr->p_flags as CH_FLAG_MODE_HEAP, so ChibiOS will call free()
   * on the memory after it exits.
   */
  ((uint8_t *)thr)[0x1d] = 1;
  return thr;
}

// You'll recognize this code from the "fade" demo!
void fade_code(void *config) {
  struct fade_thread_config *cfg = (struct fade_thread_config *)config;

  int led = cfg->pin;
  int fadeAmount = cfg->speed;

  int brightness = 0;
  while(1) {
    analogWrite(led, brightness);    // update the led's brightness

    // move the brightness by fadeAmount toward a brightness limit
    brightness = brightness + fadeAmount;

    // check if the brightness is at the bottom or top of the 0-255 scale
    // if it is, invert the amount we fade by
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    // pause for 30 milliseconds, or else the effect is too fast
    delay(30);
  }
}

void setup(void) {
  // make sure that we talk to different pins
  thread_config[0].pin = 3; // pin number can be set to any number 0-5
  thread_config[1].pin = 4;
  //            ^ this is the thread number, has to be less than NUMBER_OF_THREADS

  // configure the speed of fading
  thread_config[0].speed = 5;
  thread_config[1].speed = 8;

  // start each of the threads. We do this in setup, not loop, because
  // we don't want to keep on starting threads over and over again!

  // The first thread starts running after this line of code.
  fade_threads[0] = runThread(fade_code, &thread_config[0]);

  delay(1000); // this will delay the start of the second thread by 1 second

  // The second thread starts running after this line of code.
  fade_threads[1] = runThread(fade_code, &thread_config[1]);
}

void loop(void) {
  // since we already started two threads, the "main" thread of the program
  // is no longer needed. We exit the thread, which quits the loop and frees
  // up the Chibi Chip to focus on your new threads!
  exitThread(0);
}
