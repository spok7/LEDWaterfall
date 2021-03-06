#include "constants.h"

// interface for all animations
class Animation {
  protected:
    uint8_t funcID;
  public:
    virtual CHSV* getNext(uint8_t currID) = 0;                        // gets the next row of colours to display
};


// a dynamically-initialized animation
class Waterfall: public Animation {
  private:

    CHSV *current_colours;                              // array of colours for the current row
    int *positions;                                     // array containing position of each row in the animation
    int *random_amount;                                 // tells strip how many times to play shimmer animation before highlight
    int random_low_bound, random_high_bound;            // user-given range for generating a random value for random_amount

    struct Pulse {
      CHSV *ani;
      uint8_t len;
    } *highlight, *shimmer;                             // animation array with positions


    // populates Pulse struct with colours to created a fade-through "breathing" or "pulsing" animation
    void pulse_maker(Pulse *pulse, CHSV base_colour, CHSV target_colour, CHSV fade_rate, bool reverse_fade_rate) {

      // account for user error      
      if (fade_rate.hue == 0) fade_rate.hue == 1;
      if (fade_rate.sat == 0) fade_rate.sat == 1;
      if (fade_rate.val == 0) fade_rate.val == 1;

      // calculate animation length
      uint8_t hue_steps = ceil(abs((float) (target_colour.hue - base_colour.hue) / fade_rate.hue));
      uint8_t sat_steps = ceil(abs((float) (target_colour.sat - base_colour.sat) / fade_rate.sat));
      uint8_t val_steps = ceil(abs((float) (target_colour.val - base_colour.val) / fade_rate.val));

      // initialize Pulse values
      pulse->len = 2 * max(hue_steps, max(sat_steps, val_steps)) + 1;
      pulse->ani = new CHSV[pulse->len];


      // set fill mode based on boolean reverse_fade_rate
      int fill;
      CHSV new_target;
      if (not reverse_fade_rate) {
        fill = 0;
        pulse->ani[fill] = base_colour;
        new_target = target_colour;
      } else {
        fill = (pulse->len - 1) / 2;
        pulse->ani[fill] = target_colour;
        new_target = base_colour;
      }

      // fill half of the array
      while (pulse->ani[fill].hue != new_target.hue || pulse->ani[fill].sat != new_target.sat || pulse->ani[fill].val != new_target.val) {

        ++fill;
        
        pulse->ani[fill] = pulse->ani[fill - 1];

        if (pulse->ani[fill].hue == new_target.hue) {
          // do nothing
        } else if (abs(pulse->ani[fill].hue - new_target.hue) < fade_rate.hue){
          pulse->ani[fill].hue = new_target.hue;
        } else if (pulse->ani[fill].hue < new_target.hue) {
          pulse->ani[fill].hue += fade_rate.hue;
        } else {
          pulse->ani[fill].hue -= fade_rate.hue;
        }
      
        if (pulse->ani[fill].sat == new_target.sat) {
          // do nothing
        } else if (abs(pulse->ani[fill].sat - new_target.sat) < fade_rate.sat){
          pulse->ani[fill].sat = new_target.sat;
        } else if (pulse->ani[fill].sat < new_target.sat) {
          pulse->ani[fill].sat += fade_rate.sat;
        } else {
          pulse->ani[fill].sat -= fade_rate.sat;
        }
      
        if (pulse->ani[fill].val == new_target.val) {
          // do nothing
        } else if (abs(pulse->ani[fill].val - new_target.val) < fade_rate.val){
          pulse->ani[fill].val = new_target.val;
        } else if (pulse->ani[fill].val < new_target.val) {
          pulse->ani[fill].val += fade_rate.val;
        } else {
          pulse->ani[fill].val -= fade_rate.val;
        }
      }

      // fill other half of array by mirroring the initial half
      for (int i = 0; i < (pulse->len - 1) / 2; ++i) {
        if (not reverse_fade_rate) {
          pulse->ani[pulse->len - 1 - i] = pulse->ani[i];
        } else {
          pulse->ani[i] = pulse->ani[pulse->len - 1 - i];
        }
      }
    }

    // code for stepping through an animation array and adding randomness aspect
    void stepStrip(uint8_t strip_num) {
      if (positions[strip_num] == highlight->len + shimmer->len - 1) {
        if (random_amount[strip_num] == 1) {
          positions[strip_num] = 0;
          random_amount[strip_num] = random_low_bound + random8(random_high_bound - random_low_bound + 1);
          current_colours[strip_num] = highlight->ani[positions[strip_num]];
        } else {
          positions[strip_num] = highlight->len;
          --random_amount[strip_num];
          current_colours[strip_num] = shimmer->ani[positions[strip_num] - highlight->len];
        }
      } else if (positions[strip_num] < highlight->len) {
        current_colours[strip_num] = highlight->ani[positions[strip_num]];
      } else {
        current_colours[strip_num] = shimmer->ani[positions[strip_num] - highlight->len];
      }
    }

    // stopping animation that resets all strips instantly
    void stop_quick() {
      for (int strip_num = 0; strip_num < STRIP_AMOUNT; ++strip_num) {
        positions[strip_num] = 0;
      }
    }

  public:

    // constructor that initializes class variables and generates Pulse strips
    Waterfall(uint8_t id, CHSV base_col, CHSV highlight_col, CHSV shimmer_col, CHSV highlight_rate, CHSV shimmer_rate, bool highlight_reversed, bool shimmer_reversed, uint8_t repeat_low_bound, uint8_t repeat_high_bound) {

      funcID = id;
      
      highlight = new Pulse();
      shimmer = new Pulse();
      
      current_colours = new CHSV[STRIP_AMOUNT];
      positions = new int[STRIP_AMOUNT];
      
      random_amount = new int[STRIP_AMOUNT];
      random_low_bound = repeat_low_bound;
      random_high_bound = repeat_high_bound;

      for (int strip_num = 0; strip_num < STRIP_AMOUNT; ++strip_num) {
        positions[strip_num] = 0;
        random_amount[strip_num] = random_low_bound + random8(random_high_bound - random_low_bound + 1);
      }

      pulse_maker(highlight, base_col, highlight_col, highlight_rate, highlight_reversed);
      pulse_maker(shimmer, base_col, shimmer_col, shimmer_rate, shimmer_reversed);
    }

    CHSV* getNext(uint8_t currID) {

      if (currID == funcID) {

        // play regular animation
        for (int strip_num = 0; strip_num < STRIP_AMOUNT; ++strip_num) {
          stepStrip(strip_num);  
          ++positions[strip_num];
        }
  
      } else {

        // check if animation is complete
        bool complete = true;
        for (int strip_num = 0; strip_num < STRIP_AMOUNT; ++strip_num) {
          if (positions[strip_num] != 0) {
            complete = false;
            break;
          }
        }

        if (complete) return NULL;

        // play stopping animation
        stop_quick();      
      }

      return current_colours;
    }
};



// a hardcoded animation
class OHML: public Animation{
  
  private:
  
    int
      run_amount,
      pos,
      len;
    CHSV
      fg,
      bg,
      strip[64][STRIP_AMOUNT];
      
  public:

    // constructor that initializes class variables and creates the animation
    OHML(uint8_t id, CHSV base, CHSV background, int num_runs = 1){
      funcID = id;
      fg = base;
      bg = background;
      run_amount = num_runs;
      len = 64;
      pos = 0;

      if (STRIP_AMOUNT == 5) {
      
        CHSV temp_array[len][5] {
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
  
          //O
          {bg, fg, fg, fg, bg},
          {fg, fg, bg, fg, fg},
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
          {fg, fg, bg, fg, fg},
          {bg, fg, fg, fg, bg},
  
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
  
          //H
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
          {fg, fg, fg, fg, fg},
          {fg, fg, fg, fg, fg},
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
  
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
        
          //M
          {fg, bg, bg, bg, fg},
          {fg, fg, bg, fg, fg},
          {fg, fg, fg, fg, fg},
          {fg, bg, fg, bg, fg},
          {fg, bg, fg, bg, fg},
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
          {fg, bg, bg, bg, fg},
       
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
  
          //L
          {fg, bg, bg, bg, bg},
          {fg, bg, bg, bg, bg},
          {fg, bg, bg, bg, bg},
          {fg, bg, bg, bg, bg},
          {fg, bg, bg, bg, bg},
          {fg, bg, bg, bg, bg},
          {fg, bg, bg, bg, bg},
          {fg, fg, fg, fg, fg},
          {fg, fg, fg, fg, fg},
        
          //space
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg},
          {bg, bg, bg, bg, bg}
        };
  
        memcpy(strip, temp_array, sizeof(CHSV) * len * STRIP_AMOUNT);
      }
    }

    CHSV* getNext(uint8_t currID) {
      if (pos == 0) {
        pos = len;
        if (/*run_amount > 1 &&*/ currID == funcID) {
          --run_amount;
          return strip[--pos];
        } else{
          return NULL;
        }
      } else {
        return strip[--pos];
      }
    }
};
