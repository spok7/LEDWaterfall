#include <FastLED.h>
#include <avr/power.h>
#include "constants.h"

CRGB strip0[STRIP_LENGTH];
Waterfall *waterfall;

void setup() {
  FastLED.addLeds<APA102, STRIP0_DATA_PIN, STRIP0_CLOCK_PIN>(strip0, STRIP_LENGTH);

  //reset previous lights
  for(int i = 0; i < STRIP_LENGTH; ++i) {
    strip0[i] = 0;
  }
  FastLED.show();

//  waterfall = new Waterfall(80, 127, 64);
//  waterfall->reset();
//  waterfall->drop(0);
//  waterfall->updateFalls();
//  delay(500);
//  waterfall->drop(0);
//  waterfall->updateFalls(); 
}

//int head = 143, tail = head - 1; // the minimum value is -4

void loop() {
//  strip0[head] = CHSV(80, 127, 64);
//  strip0[tail] = 0;
//  FastLED.show();
//  if (++head >= STRIP_LENGTH) head = 0;
//  if (++tail >= STRIP_LENGTH) tail = 0;
//  delay(DELAY);

//  waterfall->drop(0);
//  FastLED.show();  
//  delay(DELAY);
}
