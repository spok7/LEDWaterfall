#define DELAY 200

#define STRIP_LENGTH 144

#define STRIP0_DATA_PIN 4
#define STRIP0_CLOCK_PIN 5

//#define STRIP1_DATA_PIN 8
//#define STRIP1_CLOCK_PIN 6
//
//#define STRIP2_DATA_PIN 1
//#define STRIP2_CLOCK_PIN 3
//
//#define STRIP3_DATA_PIN 12
//#define STRIP3_CLOCK_PIN 11
//
//#define STRIP4_DATA_PIN 13
//#define STRIP4_CLOCK_PIN 10

#ifndef _WATERFALL_H_
#define _WATERFALL_H_

class Waterfall
{
  private:

    CRGB strip[5][STRIP_LENGTH];        // the strip objects
    CRGB strip_test[STRIP_LENGTH];
    int rates[5][STRIP_LENGTH];         // matrix corresponding to LEDs on waterfall - used for colour change tracking
    
    int colour, saturation, brightness; // initial LED settings
    int fade_speed;                     // the interval by which to shift the brightness
    
  public:

    Waterfall(int colour, int saturation, int brightness);
   void
    drop(int strip_num),
    updateFalls(),
    reset();
};

#endif
    
