// Love to Code
// Basic Multithreads
//
// Multithreading is the act of running multiple pieces of code at the same time. 
//
// As long as you don't share any variables between threads, multithreading
// is straightforward. This template allows you to put code in up to six
// threads that run in parallel.
//
// The easiest way to use this code is to imagine you are writing a separate program,
// with differently named variable names and pin names by number, for each output pin on Chibi Chip.
// 
// For this code to run, we must have all six setup and loop threads in the code even if 
// they are blank.

#include "ChibiOS.h" // we need to include our special code to enable threading
#include "SimpleThreads.h" // we need to include our special code to enable threading

///// thread 0
int led0 = 0;

void setup0() {
  // thread 0's specific setup code here
  outputMode(led0);
}

void loop0() {
  // thread 0's loop code here
  int brightness0 = 0;

  while (brightness0 < 100){
    setLevel(led0, brightness0);
    pause(10);
    brightness0 = brightness0 + 1;
  }
  
  while (brightness0 > 0){
    setLevel(led0, brightness0);
    pause(10);
    brightness0 = brightness0 - 1;
  }
                          
}

///// thread 1
int led1 = 1;

void setup1() {
  // thread 1’s specific setup code here
  outputMode(led1);
}

void loop1() {
  // thread 1’s loop code here
  int brightness1 = 100;

  while (brightness1 > 0){
    setLevel(led1, brightness1);
    pause(10);
    brightness1 = brightness1 - 1;
  }
  
  while (brightness1 < 100){
    setLevel(led1, brightness1);
    pause(10);
    brightness1 = brightness1 + 1;
  }
                          
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

