#include <LocoNet.h>
#include <avr/power.h>
#include "Animation.cpp"


CRGB strip[STRIP_AMOUNT][STRIP_LENGTH];     // declares array of LED strips
bool not_visible;
uint8_t dim_current;
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

const CHSV ORANGE(150, 255, 255); // 255 (don't go below 32)
const CHSV WHITE(150, 64, 32); // 32


Waterfall *canadaDayFall;

const CHSV CD_WATER(70, 180, 32);

const CHSV CD_HIGHLIGHT(70, 180, 48);
const CHSV CD_HIGHLIGHT_RATE(0, 25, 25);

const CHSV CD_SHIMMER(70, 180, 32);
const CHSV CD_SHIMMER_RATE(25, 0, 0);


Leaf *leaf;

const CHSV RED(130, 255, 255);


/* 
 * HELPER METHODS
 * Do not use or edit.
 * ...or don't even worry about them.
 *
 */


// method controlling brightness for day / night modes based off of pin input
void dim_on_pin() {
  if (digitalRead(DAY_PIN) == HIGH && dim_current != 0) {
    dim_current = (dim_current <= DIM_RATE) ? 0 : dim_current - DIM_RATE;
  } else if (digitalRead(DAY_PIN) == LOW && dim_current != DIM_PERCENT) {
    dim_current = (DIM_PERCENT - dim_current <= DIM_RATE) ? DIM_PERCENT : dim_current + DIM_RATE;
  }
  
  for (int strip_num = 0; dim_current != 0 && strip_num < STRIP_AMOUNT; ++strip_num) {
    strip[strip_num][START].fadeLightBy(dim_current);
  }
}

// method controlling brightness for day / night modes based odd of class variable controlled by LocoNet
void dim_with_fading() {
  if (dim_current > dim_target) {
    dim_current = (dim_current - dim_target <= DIM_RATE) ? dim_target : dim_current - DIM_RATE;
  } else if (dim_current < dim_target){
    dim_current = (dim_target - dim_current <= DIM_RATE) ? dim_target : dim_current + DIM_RATE;
  }

  if(dim_current != 0) {
    for (int strip_num = 0; strip_num < STRIP_AMOUNT; ++strip_num) {
      strip[strip_num][START].fadeLightBy(dim_current);
    }
  }
}

// simplified version of method above
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
void notifyThrottleAddress(uint8_t UserData, TH_STATE State, uint16_t Address, uint8_t Slot)    {Serial.print(F("Address: "));     Serial.println(Address);}
void notifyThrottleSlotStatus(uint8_t UserData, uint8_t Status)                                 {Serial.print(F("Slot Status: ")); Serial.println(Status);}
void notifyThrottleError(uint8_t UserData, TH_ERROR Error)                                      {Serial.print(F("Error: "));       Serial.println(Throttle.getErrorStr(Error));}
void notifyThrottleState(uint8_t UserData, TH_STATE PrevState, TH_STATE State)                  {Serial.print(F("State: "));       Serial.println(Throttle.getStateStr(State));}
void notifyThrottleSpeed(uint8_t UserData, TH_STATE State, uint8_t Speed)                       {Serial.print(F("Speed: "));       Serial.println(Speed); dim_target = 2 * (127 - Speed);}
void notifyThrottleDirection(uint8_t UserData, TH_STATE State, uint8_t Direction)               {Serial.print(F("Direction: "));   Serial.println(not_visible = Direction);}
void notifyThrottleFunction(uint8_t UserData, uint8_t Function, uint8_t Value)                  {Serial.print(F("Function: "));    Serial.println(Function);
                                                                                                 animation_selector = (Function < NUM_ANIMATIONS) ? Function : animation_selector;}


// used for printing and processing LocoNet packets
void lnUpdate() {
  LnPacket = LocoNet.receive();
  if (LnPacket) {
    
    // First print out the packet in HEX
    Serial.print(F("RX: "));
    uint8_t msgLen = getLnMsgSize(LnPacket);
    for (uint8_t x = 0; x < msgLen; x++)
    {
      uint8_t val = LnPacket->data[x];
      // Print a leading 0 if less than 16 to make 2 HEX digits
      if (val < 16)
        Serial.print('0');

      Serial.print(val, HEX);
      Serial.print(' ');
    }

    // If this packet was not a Switch or Sensor Message then print a new line, and process it as a Throttle Message
    if (!LocoNet.processSwitchSensorMessage(LnPacket)) {
      Serial.println();
      Throttle.processMessage(LnPacket);
    }
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

  Serial.begin(9600);

  pinMode(DAY_PIN, INPUT);
  
  if (0 < STRIP_AMOUNT) FastLED.addLeds<APA102, STRIP0_DATA_PIN, STRIP0_CLOCK_PIN>(strip[0], STRIP_LENGTH);
  if (1 < STRIP_AMOUNT) FastLED.addLeds<APA102, STRIP1_DATA_PIN, STRIP1_CLOCK_PIN>(strip[1], STRIP_LENGTH);
  if (2 < STRIP_AMOUNT) FastLED.addLeds<APA102, STRIP2_DATA_PIN, STRIP2_CLOCK_PIN>(strip[2], STRIP_LENGTH);
  if (3 < STRIP_AMOUNT) FastLED.addLeds<APA102, STRIP3_DATA_PIN, STRIP3_CLOCK_PIN>(strip[3], STRIP_LENGTH);
  if (4 < STRIP_AMOUNT) FastLED.addLeds<APA102, STRIP4_DATA_PIN, STRIP4_CLOCK_PIN>(strip[4], STRIP_LENGTH);

  FastLED.show();   //display strips to clear previous run
  
  Serial.println(F("Initialized Strips"));
   
  // initialize Animation objects here; see possible Animations and their constructors in Animation.cpp
  waterfall = new Waterfall(0, WATER, HIGHLIGHT, SHIMMER, HIGHLIGHT_RATE, SHIMMER_RATE, false, false, 3, 7);
  ohml = new OHML(1, ORANGE, WHITE);
  canadaDayFall = new Waterfall(2, CD_WATER, CD_HIGHLIGHT, CD_SHIMMER, CD_HIGHLIGHT_RATE, CD_SHIMMER_RATE, false, false, 3, 7);
  leaf = new Leaf(3, RED, WHITE);
  
  Serial.println(F("Initialized Animations")); 
   

  // setup LocoNet and Throttle
  LocoNet.init(TX_PIN);
  Throttle.init(0, 0, LOCO_ADDRESS);

  // attempt to initialize onto existing address
  Serial.println(F("\tStealing Address"));
  Throttle.stealAddress(LOCO_ADDRESS);
  lnUpdate();     // repeated to process possible errors
  lnUpdate();     // do not remove duplicate line

  // if address does not exist, create one
  if (Throttle.getState() == TH_ST_FREE) {
    Serial.println(F("\nSetting Address"));
    Throttle.setAddress(LOCO_ADDRESS);
  }
  lnUpdate();

  Serial.println(F("Established connection with LocoNet"));


  // initialize variables that have been written over with the loconet update calls
  dim_current = 0;
  animation_selector = 2;

  Serial.println(F("Initialized Starting Configuration"));

  Serial.print(F("Memory Left: "));
  Serial.println(freeMemory());
  
  Serial.println(F("\nExiting Setup\n"));
}

// continuous loop
void loop() {
  animation_runner(waterfall);
  animation_runner(ohml);
  animation_runner(canadaDayFall);
  animation_runner(leaf);
}

