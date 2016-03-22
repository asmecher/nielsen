/**
 * Nielsen speaker controller sketch
 * Copyright (c) 2016 by Alec Smecher
 * See http://cassettepunk.com/blog/2016/03/21/nielsen-speaker/ for details.
 * See LICENSE for licensing information.
 */

#include "Keypad.h"
#include "synth.h"

#define MAX(a,b) ((a)<(b)?(b):(a))
#define MIN(a,b) ((a)>(b)?(b):(a))

#define DEBUG

void printd(char *s) {
  #ifdef DEBUG
  Serial.println(s);
  #endif
}

synth s;

/**
 * Button constants
 */
const char DO = '1';
const char RE = '2';
const char MI = '3';
const char FA = '4';
const char SO = '5';
const char LA = '6';
const char TI = '7';

const char TEMPO_D = '-';
const char TEMPO_U = '+';
const char STOP = 's';
const char SFX = 'S'; // sustain
const char DEMO = 'D'; // chord

const char KEY8 = 'a';
const char KEY9 = 'b';
const char KEY10 = 'c';
const char KEY11 = 'd';
const char KEY12 = 'e';
const char KEY13 = 'f';

const char WHAMMY = 'w';

// Voice constants
const char MOTOR_VOICE = 0;
const char TONE_VOICE = 1;

// Keypad configuration
const byte ROWS = 3;
const byte COLS = 7;
char keys[ROWS][COLS] = {
  {KEY8, KEY9, KEY11, KEY10, KEY12, KEY13, 'x'},
  {TEMPO_D, TEMPO_U, STOP, SFX, DEMO, WHAMMY, 'y'},
  {TI, LA, SO, FA, MI, RE, DO}
};
byte rowPins[ROWS] = {2, 4, 6};
byte colPins[COLS] = {7, 8, 9, 10, 12, 13, A0};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LED pins (set pin low to turn on)
const int LED0PIN = A2;
const int LED1PIN = A3;
const int LED2PIN = A4;
const int LED3PIN = A5;

// Volume knob analog input
const int VOLPIN = A1;

// Current "sustain" flag state
byte sustain = true;

void setup() {
  #ifdef DEBUG
    Serial.begin(9600);
  #endif

  // Initialize the synthesizer
  s.begin();
  s.setupVoice(MOTOR_VOICE,SQUARE,60,ENVELOPE1,127,64);
  s.setupVoice(TONE_VOICE,SAW,60,ENVELOPE1,127,64);

  // Initialize LED pins
  pinMode(LED0PIN, OUTPUT); digitalWrite(LED0PIN, HIGH);
  pinMode(LED1PIN, OUTPUT); digitalWrite(LED1PIN, HIGH);
  pinMode(LED2PIN, OUTPUT); digitalWrite(LED2PIN, HIGH);
  pinMode(LED3PIN, OUTPUT); digitalWrite(LED3PIN, HIGH);

  // Set up the volume knob analog input pin
  pinMode(VOLPIN, INPUT);
  digitalWrite(VOLPIN, HIGH); // Set pull-up

  // Set "power" light on
  digitalWrite(LED3PIN, LOW);
}

uint16_t f; // Current motor frequency
char octave = 0; // Current octave (midi octave + 1)
bool chord = 0; // Chording mode flag

// Play a MIDI note according to the current flags.
void doNote(unsigned char mnote) {
  setLength();
  s.mTrigger(MOTOR_VOICE, mnote);
  if (chord) s.mTrigger(TONE_VOICE, mnote+7);
}

// Set the note length according to the current flags.
void setLength() {
  s.setLength(MOTOR_VOICE, sustain?127:110);
  s.setLength(TONE_VOICE, sustain?127:100);
}

unsigned char i=0;

void loop()
{
  i++; // loop counter

  // Don't always read the volume pin -- it's slow.
  if (i==0) {
    unsigned int vol = analogRead(VOLPIN);
    int mod = vol>1000 ? 64 : MAX(0, MIN(127, (vol - 16) * 3));
    digitalWrite(LED2PIN, mod != 64); 
    s.setMod(MOTOR_VOICE, mod);
    s.setMod(TONE_VOICE, mod);
  }
    
  char key = keypad.getKey();
  if (key != NO_KEY || (Serial.available()>0 && (key=Serial.read()))) switch (key) {
    case DO: doNote(0+(octave*12)); break;
    case RE: doNote(2+(octave*12)); break;
    case MI: doNote(4+(octave*12)); break;
    case FA: doNote(5+(octave*12)); break;
    case SO: doNote(7+(octave*12)); break;
    case LA: doNote(9+(octave*12)); break;
    case TI: doNote(11+(octave*12)); break;
    case TEMPO_D: // Drop an octave
      octave=MAX(0, octave-1);
      break;
    case TEMPO_U: // Raise an octave
      octave=MIN(9, octave+1);
      break;
    case STOP: // Stop both voices
      s.setLength(MOTOR_VOICE, 0);
      s.setLength(TONE_VOICE, 0);
      break;
    case SFX: // Invert sustain flag
      digitalWrite(LED0PIN, sustain);
      sustain = !sustain;
      setLength();
      break;
    case DEMO: // Invert chord flag
      digitalWrite(LED1PIN, chord);
      chord = !chord;
      break;
    case WHAMMY: // Do a frequency sweep
      for (int j=0; j<3000; j++) {
        s.setFrequency(MOTOR_VOICE, j);
        delay(20);
      }
      break;
    // KEY8 through KEY13 set voice characteristics
    case KEY8: s.setWave(MOTOR_VOICE,SINE); break;
    case KEY9: s.setWave(MOTOR_VOICE,SQUARE); break;
    case KEY10: s.setWave(MOTOR_VOICE,SAW); break;
    case KEY11: s.setWave(TONE_VOICE,SINE); break;
    case KEY12: s.setWave(TONE_VOICE,NOISE); break;
    case KEY13: s.setWave(TONE_VOICE,RAMP); break;
    #ifdef DEBUG
      default: Serial.println(key);
    #endif
  }
}
