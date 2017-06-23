// Love to Code

// Music and light effects template: play music on one pin while flashing lights on the others
// Music code retrieved from http://www.instructables.com/id/Arduino-Tone-Music/
// Special thanks to ShiCheng Lim
      
// Effects Template
enum effects {  CONSTANT = 0,  FADE = 1,  HEARTBEAT = 2,  TWINKLE = 3 }; // don't delete this line!

////// configuration /////////
////// PICK YOUR EFFECT AND SPEED HERE
////// choose one of CONSTANT, FADE, HEARTBEAT, TWINKLE
////// and a speed from 1 to 25 (higher is faster)
////// See advanced_settings() below for more options
int pin0_effect = HEARTBEAT;
int pin0_speed = 4;

int pin1_effect = HEARTBEAT;
int pin1_speed = 1;

int pin2_effect = TWINKLE;
int pin2_speed = 3;

int pin3_effect = FADE;
int pin3_speed = 4;

int pin4_effect = FADE;
int pin4_speed = 3;

int pin5_effect = HEARTBEAT;
int pin5_speed = 5;

//////////// Music Template
int speaker_pin = 3;  // Speaker connected to which pin?
// note that whichever pin is selected for music will automatically have its effect removed

int octave = 4; // note octave (initialized at octave 4)
int dur = 1500/4; // note duration (initialized at quarter note duration)
int music_pause = 2; // number of seconds to wait before looping the song

const char tune[] = // happy birthday!
		   "r r 4a4. a4 b4 a4 5d4 c#2 "
		   "4a4. a4 b4 a2 5e4 d4 "
		   "4a4. a4 5a f#4 d4 c#4 4b4 "
		   "5g4. g4 f#4 d4 e4 d2 ";

/*  _______________________________________________
    | MUSIC NOTATION FORMAT                        |
    | Example: 3c#4.                               |
    | means 3 octave, C sharp quarter note, dotted |   
    |                                              |
    | 1 = whole note, 2= 1/2, 4 = 1/4, 8 = 1/8,    |
    | 16 = 1/16   Dotted notes are duration + 1/2  |
    |                                              |
    | RESTS: Example r4 = quarter rest etc.        |
    |                                              |
    | Note: it is not necessary to enter octave or |
    | note duration for each note, unless the      |
    | octave or duration changes from the previous |
    | note.                                        |
    |______________________________________________|*/





#include "Arduino.h"
#include "ChibiOS.h" // needed for multithreading
#include <stdio.h>  // standard library for input / output
#include <string.h> // functions for string operations 
#include <ctype.h>  // functions that operate on single byte characters


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

// ARRAY OF FREQUENCIES OF MUSIC NOTES AT THE 7TH OCTAVE
// (FREQUENCY OF 0 ADDED FOR RESTS)
const int frequencies[] = {2093, 2349, 2637, 2794, 3136, 3520, 3951, 0};

// DIVISORS USED TO PRODUCE MUSIC NOTE FREQUENCIES AT OCTAVES 1 TO 7 
// (1 IS DIVISOR FOR RESTS)
const int divisors[] = {128,64,32,16,8,4,2,1}; 


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

      /*_____________________________________________________________________________
        |                     noteIndex FUNCTION                                     |
        | Given a character representing a note, returns the position in the octave. |
        | Example: 'C' => 0, 'D' => 1, 'E' => 2, plus 'R' for rest  => 1             |
        | Returns -1 on error                                                        |
        |____________________________________________________________________________|*/
        
int noteIndex(char note)
{
  const char notes[] = {'C', 'D', 'E', 'F', 'G', 'A', 'B', 'R'};
  const int noteCount = 8;
  for (int i = 0; i < noteCount; i++)
  {
    if (toupper(note) == notes[i])
      {
         return i;  // Found it at position i
      }
   }
  return -1;  // didn't find it
} // end of noteIndex function

/*  ___________________________________________
   |         playMusic FUNCTION                |
   | This function parses the playMusic String |
   | of notes, selects each segment separated  |
   | by spaces and sends each segment in       |
   | sequence to the function playNote()       |
   |___________________________________________|*/

void playMusic(String music)  
{
  int curPos=0;     // current position in String
  int nextSpacePos; // position of next space character
  
  // While there is at least one more space in the string, 
  // select the String segment before the space 
  // and send it to playNote();
  nextSpacePos = music.indexOf(" ",curPos);

  while (nextSpacePos != -1)
  {
    if (nextSpacePos > curPos) // don't send zero-length segment
    {
    // There is at least one character before the space so 
    // send it to playNote()
      String note = music.substring(curPos, nextSpacePos);
      playNote(note);
     }
    
    // Move the current position forward and check for another space
    curPos = nextSpacePos + 1;
    nextSpacePos = music.indexOf(" ", curPos);
  }
  
    // If there are any characters at the end of the string after
    // the last space, send those to playNote() too
     if (curPos < (music.length() - 1))
     {
       String note = music.substring(curPos);
       playNote(note);
      }
  return;
} //  end of playMusic() function            -
           

/*  _______________________________________________
    |         playNote() FUNCTION                  |
    | Each note is the space delineated segment    |
    | selected by the function playMusic()         |
    |                                              |
    | MUSIC NOTATION FORMAT                        |
    | Example: 3c#4.                               |
    | means 3 octave, C sharp quarter note, dotted |   
    |                                              |
    | 1 = whole note, 2= 1/2, 4 = 1/4, 8 = 1/8,    |
    | 16 = 1/16   Dotted notes are duration + 1/2  |
    |                                              |
    | RESTS: Example r4 = quarter rest etc.        |
    |                                              |
    | playNote() adjusts the note frequency        |
    | for octaves, accidentals (#)                 |
    | and dotted notes.                            |
    | Note: it is not necessary to enter octave or |
    | note duration for each note, unless the      |
    | octave or duration changes from the previous |
    | note.                                        |
    |______________________________________________|*/
    
void playNote(String music)
{  
// CHECK NOTE SEGMENT FOR VALID LENGTH
   int len = music.length();
   if ((len < 1) || (len > 6)) 
    {
       return;
    }

//GET OCTAVE IF IT HAS CHANGED
    while (isdigit(music.charAt(0)))
      {
        octave = music.charAt(0);  
        octave = octave - 48; // converts ASCII value to octave number
        music = music.substring(1); // DROP OCTAVE FROM music STRING   
      }
      
// GET NOTE INDEX (includes rests)
    int index = noteIndex(music.charAt(0));
      if (index == -1)
      { 
        return; 
      }
      music = music.substring(1); // DROP NOTE FROM music STRING
      
// CALCULATE FREQUENCY
    int frequency = frequencies[index];
    frequency = frequency / divisors[octave];

// ADJUST FREQUENCY FOR ACCIDENTALS (SHARP # or + AND FLAT -)    
    if ((music.charAt(0) == '#') || (music.charAt(0) =='+')) 
      { 
        frequency *= 1.059;
        music = (music.substring (1)); // DROP '#' or '+' FROM music STRING
      }
    if (music.charAt(0) == '-')
      {
        frequency /= 1.059;
        music = (music.substring (1)); // DROP FLAT '-' FROM music STRING
      }  
      
// GET DURATION
  if ((music.toInt()>0) && (music.toInt() <17)) // IF IN RANGE OF WHOLE NOTE TO SIXTEENTH NOTE
  { 
    dur = (music.toInt());
    dur = 1800 / dur; // whole note => 1 1/2 sec. (change to change tempo)
  }
   
    
// ADJUST DURATION IF DOTTED NOTE    
    if (music.endsWith ("."))
      {
        dur = dur * 1.5; 
      }  
  
  // ADJUST DURATION IF ~ NOTE
  if (music.endsWith ("~"))
      {
        dur = dur * 0.5; 
      }  
      
       
 // PLAY THE NOTE   
    tone(speaker_pin, frequency, dur);  
    delay(dur);

}//   end of playNote() function

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
    
    if( i == speaker_pin )
      continue; // skip making a thread for the speaker pin
    
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
  
  Serial.begin(9600); // in case we need debugging with a Chibi Scope
  pinMode(speaker_pin, OUTPUT);
}

void loop(void) {
  playMusic(String(tune));
  delay(music_pause * 1000);
}

