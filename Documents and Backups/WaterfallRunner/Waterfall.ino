#include <FastLED.h>
#include "constants.h"

Waterfall::Waterfall(int colour, int saturation, int brightness)
{
  FastLED.addLeds<APA102, STRIP0_DATA_PIN, STRIP0_CLOCK_PIN>(strip_test, STRIP_LENGTH);
//      FastLED.addLeds<APA102, STRIP1_DATA_PIN, STRIP1_CLOCK_PIN>(strip[1], STRIP_LENGTH);
//      FastLED.addLeds<APA102, STRIP2_DATA_PIN, STRIP2_CLOCK_PIN>(strip[2], STRIP_LENGTH);
//      FastLED.addLeds<APA102, STRIP3_DATA_PIN, STRIP3_CLOCK_PIN>(strip[3], STRIP_LENGTH);
//      FastLED.addLeds<APA102, STRIP4_DATA_PIN, STRIP4_CLOCK_PIN>(strip[4], STRIP_LENGTH);

  // initialize rates
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < STRIP_LENGTH; ++j) {
      rates[i][j] = 0;
    }
  }

  this->colour = colour;
  this->saturation = saturation;
  this->brightness = brightness;
}

// causes a stream to flow down one of the strips
void Waterfall::drop(int strip_num)
{
  strip_test[0] = CHSV(colour, saturation, brightness);
  rates[strip_num][0] = 1;
  updateFalls();
  delay(800);
  strip_test[0] = 0;
  updateFalls();
}

void Waterfall::updateFalls()
{
  FastLED.show();
}

void Waterfall::reset()
{
  for(int i = 0; i < 46; i++) {
    strip_test[i] = CHSV(colour, saturation, brightness);
    updateFalls();
    delay(20);
    strip_test[i] = 0;
    updateFalls();
    delay(20);
  }
}

