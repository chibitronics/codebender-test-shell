// Love to Code

// Simple multithread template
//
// Multithreading is the act of running multiple pieces of code at the same time.
// Concurrency is the act of sharing variables between threads.
//
// Remember: Multithreading is easy; concurrency is hard.
//
// So, as long as you don't share any variables between threads, multithreading
// is straightforward. This template allows you to put code in up to six
// threads that execute in parallel.
//
// The easiest way to use this code is to imagine you are writing a separate program,
// with differently named variable names and pin names, for each output pin on LTC.
//
// If you put "tN_" (e.g. t0_, t1_, t2_) in front of every variable name within
// each thread, it will help make sure you don't accidentally share a simple variable
// name between threads.

#include "ChibiOS.h" // we need to include our special functions to enable threading
#include "SimpleThreads.h"

const int g_fadeAmount = 5; // this is global, mark as const to avoid modifications
// "const" means a variable can only be read. Read-only variables can be safely shared
// across all threads.

///// thread 0
int t0_brightness = 0;
int t0_led = 0;
int t0_fadeAmount;

void setup0() {
  // thread 0's specific setup stuff here
  pinMode(t0_led, OUTPUT);
  t0_fadeAmount = g_fadeAmount;
}

void loop0() {
  // thread 0's loop here
  analogWrite(t0_led, t0_brightness);

  t0_brightness = t0_brightness + t0_fadeAmount;

  if (t0_brightness <= 0 || t0_brightness >= 255) {
    t0_fadeAmount = -t0_fadeAmount ; 
  }     

  delay(30);                            
}

///// thread 1
int t1_brightness = 128;
int t1_led = 1;
int t1_fadeAmount;

void setup1() {
  // thread 1's specific setup stuff here
  pinMode(t1_led, OUTPUT);
  t1_fadeAmount = g_fadeAmount;
}

void loop1() {
  // thread 1's loop here
  analogWrite(t1_led, t1_brightness);

  t1_brightness = t1_brightness + t1_fadeAmount;

  if (t1_brightness <= 0 || t1_brightness >= 255) {
    t1_fadeAmount = -t1_fadeAmount ; 
  }     

  delay(30);                            
}


///// thread 2 
void setup2() {
  outputMode(2);
}

void loop2() {
  on(2);
  pause(500);
  off(2);
  pause(500);
}

///// thread 3 
void setup3() {
  outputMode(3);
}

void loop3() {
  on(3);
  pause(400);
  off(3);
  pause(400);
}

///// thread 4 
void setup4() {
  outputMode(4);
}

void loop4() {
  on(4);
  pause(600);
  off(4);
  pause(600);
}

///// thread 5 
void setup5() {
  outputMode(5);
}

void loop5() {
  on(5);
  pause(1000);
  off(5);
  pause(1000);
}

