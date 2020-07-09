
#ifndef Chip_Timing_library_h
#define Chip_Timing_library_h

#include "Arduino.h"


class cellTest {


public:
  cellTest(char row, char column, char final);

  void testCell(char row, char column, char final);

  void cell_setup();

  void source_on();

  void start_chip();

  void update_valves();

  void set_all_valves(bool state);

  void set_valve(char valve_pair[], bool val);

  void trigger_setup(bool final_state);

  void trigger_pulse();

  void trigger_hold(bool final_state);

  void end_pulse();

  void reset(boolean serial_out);

  void timing_trigger(signed long ts, signed long tpw, signed long th);

  const char hi_pressure = 17;

private:


  float source_pressure = 0;
  const char source_pressure_input = 5;
  float manifold_pressure = 0;
  const char manifold_pressure_input = 4;

  //pressure variables
  const int gauge[6] = {0, 1, 2, 3, 4, 5}; // set pins according to appropriate ANALOG IN pins on Arduino


  float lin_slope[6] = {0, 0, 0, 0, 1 / (.8 * 5 / 100) * 5 / 1023, 1 / .0667 * 5 / 1023};
  float lin_yint[6] = {0, 0, 0, 0, -.5 / (.8 * 5 / 100), -.5 / .0667};

  //int num_gauges = sizeof(gauge) / sizeof(gauge[0]);

  bool final_state = 1;

  unsigned long current_time;
  unsigned long start_time;


  //arduino pin assignments
  const int led = 8;

  const char SER_Pin   = 7;  // data
  const char SRCLK_Pin = 6;  // clock

  const char RCLK_Pin  = 5;  // latch
  const char CLR_Pin   = 13;  // clr (clears register - active LOW)
  const char G_Pin     = 12;  // output gate (output active when G=LOW)
  const char G_LED_Pin = 11;  // output gate (output active when G=LOW


    //solenoid bank and manifold assignments
    #define valves_per_manifold 8
    char valve[8]; // set of 8 bytes stored for valve manifolds

    const char source[2] = {3, 7};

    // Define the outputs of the manifolds
    const char row[15][2] = {{0, 0}, {0, 1}, {1, 7}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}};
    const char col[15][2] = {{2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}};

    const char brightness = 240;  // brightness of LEDs, 0 = fully ON :: 255 = fully off

    const char regulator = 3;

    const char num_manifolds = 4;
    //byte num_gauges = 0;
    //boolean regulator_control = 0;

    //current row and column being tested
    char test_row;
    char test_col;

    void data_on();

    void data_off();

    void latch_on();

    void latch_off();

  };

  #endif
