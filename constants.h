#include <FastLED.h>

#define STRIP_LENGTH 144
#define STRIP_AMOUNT 5
#define START 0               // position from where the water flows from

#define NUM_ANIMATIONS 4

#define DIM_PERCENT (65 * 2.56)
#define DIM_RATE (5 * 2.56)
#define DAY_PIN 21

#define STRIP0_DATA_PIN 22
#define STRIP0_CLOCK_PIN 23

#define STRIP1_DATA_PIN 24
#define STRIP1_CLOCK_PIN 25

#define STRIP2_DATA_PIN 26
#define STRIP2_CLOCK_PIN 27

#define STRIP3_DATA_PIN 28
#define STRIP3_CLOCK_PIN 29

#define STRIP4_DATA_PIN 30
#define STRIP4_CLOCK_PIN 31


#define TX_PIN 46             // RX_PIN is 8 on Uno, and 48 on MEGA (hardcoded)
#define LOCO_ADDRESS 9001
