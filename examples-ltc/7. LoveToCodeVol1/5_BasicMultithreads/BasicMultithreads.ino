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
// For this code to run, we must have all six setup0 through setup5 and loop0 through loop5 functions in the code even if 
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
  on(led0);
  pause(300);
  off(led0);
  pause(300);                     
}

///// thread 1
void setup1() {
  outputMode(1);
}

void loop1() {
  on(1);
  pause(500);
  off(1);
  pause(500);                      
}


///// thread 2 
void setup2() {
  outputMode(2);
}

void loop2() {
  on(2);
  pause(800);
  off(2);
  pause(800);
}

///// thread 3 
void setup3() {
  outputMode(3);
}

void loop3() {
  on(3);
  pause(1000);
  off(3);
  pause(1000);
}

///// thread 4 
int led4 = 4;
void setup4() {
  outputMode(4);
}

void loop4() {
  int brightness4 = 0;  

  while(brightness4 < 100) {
    setLevel(led4, brightness4); 
    pause(10);                
    brightness4 = brightness4 + 1;
  } 
  
  
while(brightness4 > 0) {
    setLevel(led4, brightness4); 
    pause(10);                
    brightness4 = brightness4 - 1;
  }
}
///// thread 5 
int led5 = 5;
void setup5() {
  outputMode(5);
}

void loop5() {
  int brightness5 = 100;  

  while(brightness5 > 0) {
    setLevel(led5, brightness5); 
    pause(10);                
    brightness5 = brightness5 - 1;
  } 
  
  
while(brightness5 < 100) {
    setLevel(led5, brightness5); 
    pause(10);                
    brightness5 = brightness5 + 1;
  }
}