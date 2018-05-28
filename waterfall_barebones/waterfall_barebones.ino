#include <LocoNet.h>
#include <avr/power.h>
#include "Animation.cpp"


CRGB strip[STRIP_AMOUNT][STRIP_LENGTH];     // declares array of LED strips
bool not_visible;
uint8_t dim_target;
uint8_t animation_selector;

lnMsg *LnPacket;
LocoNetThrottleClass Throttle;


/* 
 * Animationa declaration and related colours go here.
 * And yes, throughout this code I chose to use the Canadian spelling of colour. Sorry.
 * 
 */

// HSV values: H: Hue, S: Saturation (whiteness), V: Value (brighness)

Waterfall *waterfall;

const CHSV WATER(70, 180, 32);

const CHSV HIGHLIGHT(70, 90, 48);
const CHSV HIGHLIGHT_RATE(0, 15, 2);

const CHSV SHIMMER(20, 180, 32);
const CHSV SHIMMER_RATE(4, 0, 0);


OHML *ohml;

const CHSV ORANGE(150, 255, 255);
const CHSV WHITE(150, 64, 32);


/* 
 * HELPER METHODS
 * Do not use or edit.
 * ...or don't even worry about them.
 *
 */


// sets dim level
void dim_quick() {
  if(dim_target != 0) {
    for (int strip_num = 0; strip_num < STRIP_AMOUNT; ++strip_num) {
      strip[strip_num][START].fadeLightBy(dim_target);
    }
  }
}

// shifts every light down to create illusion of flow
void flow(int wait_time) {

  // if the direction in the software is backwards, turn off the flow; the animations will continue running but they will not be visible
  if (not_visible) {
    for (int strip_num = 0; strip_num < STRIP_AMOUNT; ++strip_num) {
      strip[strip_num][START] = 0;
    }
  }
  
  dim_quick();
  FastLED.show();
  
  for (int strip_num = 0; strip_num < STRIP_AMOUNT; ++strip_num) {
    for (int pos = STRIP_LENGTH - 1; pos > START; --pos) {
      strip[strip_num][pos] = strip[strip_num][pos - 1];
    }
  }
  
  delay(wait_time);
}


/*
 * Scary LocoNet stuff.
 * 
 */


// user-defined callback functions
void notifyThrottleSpeed(uint8_t UserData, TH_STATE State, uint8_t Speed)                     {dim_target = 2 * (127 - Speed);}
void notifyThrottleDirection(uint8_t UserData, TH_STATE State, uint8_t Direction)             {not_visible = Direction;}
void notifyThrottleFunction(uint8_t UserData, uint8_t Function, uint8_t Value)                {animation_selector = (Function < NUM_ANIMATIONS) ? Function : 0;}


// used for printing and processing LocoNet packets
void lnUpdate() {
  LnPacket = LocoNet.receive() ;
  if (LnPacket && !LocoNet.processSwitchSensorMessage(LnPacket)) {
    Throttle.processMessage(LnPacket);
  }
}

/*
 * RUNNABLE FUNCTIONS
 * Feel free to use, but do not edit.
 */

// runs an animation with a given delay; default is no delay
void animation_runner(Animation *a1, int wait_time = 0) {
  CHSV *row = a1->getNext(animation_selector);
  while(row != NULL) {
    for (int strip_num = 0; strip_num < STRIP_AMOUNT; ++strip_num) {
      strip[strip_num][START] = row[strip_num];
    }
    flow(wait_time);
    row = a1->getNext(animation_selector);

    lnUpdate();
  }
}


/*
 * DEFAULT ARDUINO METHODS
 * Feel free to edit at will.
 * 
 */


// initial setup function
void setup() {

  // initialization of hardware and background variables (try not to edit this)
  
  if (0 < STRIP_AMOUNT) FastLED.addLeds<APA102, STRIP0_DATA_PIN, STRIP0_CLOCK_PIN>(strip[0], STRIP_LENGTH);
  if (1 < STRIP_AMOUNT) FastLED.addLeds<APA102, STRIP1_DATA_PIN, STRIP1_CLOCK_PIN>(strip[1], STRIP_LENGTH);
  if (2 < STRIP_AMOUNT) FastLED.addLeds<APA102, STRIP2_DATA_PIN, STRIP2_CLOCK_PIN>(strip[2], STRIP_LENGTH);
  if (3 < STRIP_AMOUNT) FastLED.addLeds<APA102, STRIP3_DATA_PIN, STRIP3_CLOCK_PIN>(strip[3], STRIP_LENGTH);
  if (4 < STRIP_AMOUNT) FastLED.addLeds<APA102, STRIP4_DATA_PIN, STRIP4_CLOCK_PIN>(strip[4], STRIP_LENGTH);

  FastLED.show();   //display strips to clear previous run
  
  
  // initialize Animation objects here; see possible Animations and their constructors in Animation.cpp
  waterfall = new Waterfall(0, WATER, HIGHLIGHT, SHIMMER, HIGHLIGHT_RATE, SHIMMER_RATE, false, false, 3, 7);
  ohml = new OHML(1, ORANGE, WHITE);
   

  // setup LocoNet and Throttle
  LocoNet.init(TX_PIN);
  Throttle.init(0, 0, LOCO_ADDRESS);

  // attempt to initialize onto existing address
  Throttle.stealAddress(LOCO_ADDRESS);
  lnUpdate();     // repeated to process possible errors
  lnUpdate();     // do not remove duplicate line

  // if address does not exist, create one
  if (Throttle.getState() == TH_ST_FREE) {
    Throttle.setAddress(LOCO_ADDRESS);
  }
  lnUpdate();


  // initialize variables that have been written over with the loconet update calls
  animation_selector = 0;
}

// continuous loop
void loop() {
  animation_runner(waterfall);
  animation_runner(ohml);
}

