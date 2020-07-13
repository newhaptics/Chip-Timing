#include "Arduino.h"
#include "Chip_Timing_library.h"

cellTest::cellTest(int row, int column, int final_state1, int final_state2) {
  this->test_row = row;
  this->test_col = column;
  this->final_state1 = final_state1;
  this->final_state2 = final_state2;
}

void cellTest::cell_setup(){
  pinMode(led, OUTPUT);

  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  pinMode(CLR_Pin, OUTPUT);
  pinMode(G_Pin, OUTPUT);
  pinMode(G_LED_Pin, OUTPUT);

  pinMode(regulator, OUTPUT);

  digitalWrite(CLR_Pin, HIGH);

  digitalWrite(G_Pin, LOW);
  analogWrite(G_LED_Pin, 250);
}

void cellTest::source_on(){
  set_valve(source, 1);
}

void cellTest::start_chip(){


  Serial.println();
  Serial.println(F("Turn on 24 V supply to solenoid valves."));
  this->source_on();
  Serial.println(F("Source is turned ON."));
  Serial.print(F("After pressing 'enter', increase Source pressure to ")); Serial.print(this->hi_pressure);
  Serial.println(F(" psi using precision regulator."));
  while (Serial.read() != 10); //wait for user to press enter.
  while (source_pressure < 15) {
    this->source_pressure = analogRead(this->source_pressure_input) * this->lin_slope[this->source_pressure_input] + lin_yint[this->source_pressure_input];
    this->manifold_pressure = analogRead(this->manifold_pressure_input) * this->lin_slope[this->manifold_pressure_input] + lin_yint[this->manifold_pressure_input];
    Serial.print("Manifold Pressure: "); Serial.print(manifold_pressure); Serial.print(" psi --- ");
    Serial.print("Source Pressure: "); Serial.print(source_pressure); Serial.println(" psi  <-- Increase Source Pressure to 16 psi");
  }
}

void cellTest::testCell(int row, int column, int final_state1, int final_state2) {
  this->test_row = row;
  this->test_col = column;
  this->final_state1 = final_state1;
  this->final_state2 = final_state2;
}

// --------------- update_valves --------------- //
// Pushes byte array "valve" to shift registers to set the state of the electronic valves
void cellTest::update_valves() {
  digitalWrite(RCLK_Pin, LOW);

  for (int i = num_manifolds - 1; i >= 0; i--) {
    shiftOut(SER_Pin, SRCLK_Pin, LSBFIRST, valve[i]);
  }
  digitalWrite(RCLK_Pin, HIGH);

}

// --------------- set_all_valves --------------- //
// Choose to set state of all valves either HIGH or LOWs
void cellTest::set_all_valves(bool state) {
  for (int i = 0; i < num_manifolds; i++) {
    if (state) this->valve[i] = 255;
    else this->valve[i] = 0;
  }
}


// --------------- set_valve --------------- //
// Takes as input valve pair indicated manifold and valve in that manifold
void cellTest::set_valve(byte valve_pair[], bool val) {
  bitWrite(this->valve[valve_pair[0]], 7 - valve_pair[1], val);
  update_valves();
}


// ---------------- latch and data functions -----------//
//turns on data of the current cell
void cellTest::data_on() {
  set_valve(col[test_col], 1);
}

//turns off data of the current cell
void cellTest::data_off() {
  set_valve(col[test_col], 0);
}

//turns on latch of current cell
void cellTest::latch_on() {
  set_valve(row[test_row], 1);
}

//turns off latch of current cell
void cellTest::latch_off() {
  set_valve(row[test_row], 0);
}

// ----------- timing functions ---------//

//tests the setup time by changing data to the final state
void cellTest::trigger_setup(bool final_state) {
  if (final_state) {
    data_on();
  } else {
    data_off();
  }
  //  Serial.println(F("setup trigger!"));
}


//starts the edge of the pulse
void cellTest::trigger_pulse() {
  latch_off();
  //  Serial.println(F("pulse trigger!"));
}

//tests the hold time by changing data to the opposite of final state
void cellTest::trigger_hold(bool final_state) {
  if (final_state) {
    data_off();
  } else {
    data_on();
  }
  //  Serial.println(F("hold trigger!"));
}

//ends the pulse
void cellTest::end_pulse() {
  latch_on();
  //  Serial.println(F("end pulse!"));
}



void cellTest::reset(boolean serial_out) {
  if (!serial_out) Serial.println(F("Resetting..."));
  set_valve(row[test_row], 0);
  set_valve(col[test_col], !final_state1);
  set_valve(col[test_col + 1], !final_state2);
  delay(250);
  set_valve(row[test_row], 1);
  delay(250);
  //  set_valve(col[test_col],final_state);
  if (!serial_out) Serial.println(F("Reset complete."));
}


//Sets up a test with a specified setup time, hold time, and pulse width
//ts is setup time; tpw is the pulse width time; th is the hold time
//takes in microsecond delay
void cellTest::timing_trigger(signed long ts, signed long tpw, signed long th) {
  //boolean values to determine if each pulse has ran
  bool setup_ran = false;
  bool pulse_start = false;
  bool pulse_end = false;
  bool hold_ran = false;

  //determine trigger times relative to the negative edge of pulse
  unsigned long elapsed_time;
  unsigned long pw_trigger_time = 50;
  unsigned long setup_trigger_time = pw_trigger_time - ts;
  unsigned long hold_trigger_time = pw_trigger_time + th;
  unsigned long pw_end_time = pw_trigger_time + tpw;

  //  Serial.println(pw_trigger_time);
  //  Serial.println(setup_trigger_time);
  //  Serial.println(hold_trigger_time);
  //  Serial.println(pw_end_time);

  digitalWrite(led, LOW);
  this->start_time = micros();

  //maybe change? start time after delay

  //delay on the front end
  //delay(100);

  //wait for time to elapse and trigger elements on the proper times
  while (!(setup_ran && pulse_start && pulse_end && hold_ran)) {
    elapsed_time = micros() - start_time;


    //trigger setup change relative to pulse width trigger
    if ((elapsed_time >= setup_trigger_time) && !setup_ran) {
      trigger_setup(final_state1);
      setup_ran = true;
      Serial.print("time elapsed for data line start "); Serial.print(elapsed_time / 1000);
      Serial.print(" milliseconds");
      Serial.println();
    }

    //trigger the pulse at the trigger time
    if ((elapsed_time >= pw_trigger_time) && !pulse_start) {
      trigger_pulse();
      pulse_start = true;
      Serial.print("time elapsed for gate line start "); Serial.print(elapsed_time / 1000);
      Serial.print(" milliseconds");
      Serial.println();
    }

    //trigger hold change after hold time
    if ((elapsed_time >= hold_trigger_time) && !hold_ran) {
      trigger_hold(final_state1);
      hold_ran = true;
      Serial.print("time elapsed for data line end "); Serial.print(elapsed_time / 1000);
      Serial.print(" milliseconds");
      Serial.println();
    }

    //end pulse after pulse trigger time
    if ((elapsed_time >= pw_end_time) && !pulse_end) {
      end_pulse();
      pulse_end = true;
      Serial.print("time elapsed for gate line end "); Serial.print(elapsed_time / 1000);
      Serial.print(" milliseconds");
      Serial.println();
    }

  }

  //delay longer on the backend if the final state is 0
  // if (final_state) {
  //   delay(200);
  // } else {
  //   delay(600);
  // }

  //all triggers done
  digitalWrite(led, HIGH);

}

//Same as timing trigger but dual
//ts is setup time; tpw is the pulse width time; th is the hold time
//takes in microsecond delay
void cellTest::dual_timing_trigger(signed long ts, signed long tpw, signed long th){
  //boolean values to determine if each pulse has ran
  bool setup_ran = false;
  bool pulse_start = false;
  bool pulse_end = false;
  bool hold_ran = false;

  //determine trigger times relative to the negative edge of pulse
  unsigned long elapsed_time;
  unsigned long pw_trigger_time = 50;
  unsigned long setup_trigger_time = pw_trigger_time - ts;
  unsigned long hold_trigger_time = pw_trigger_time + th;
  unsigned long pw_end_time = pw_trigger_time + tpw;

  //  Serial.println(pw_trigger_time);
  //  Serial.println(setup_trigger_time);
  //  Serial.println(hold_trigger_time);
  //  Serial.println(pw_end_time);

  digitalWrite(led, LOW);
  this->start_time = micros();

  //maybe change? start time after delay

  //delay on the front end
  //delay(100);

  //wait for time to elapse and trigger elements on the proper times
  while (!(setup_ran && pulse_start && pulse_end && hold_ran)) {
    elapsed_time = micros() - start_time;


    //trigger setup change relative to pulse width trigger
    if ((elapsed_time >= setup_trigger_time) && !setup_ran) {
      trigger_setup(final_state1);
      test_col++;
      trigger_setup(final_state2);
      test_col--;
      //add one to column and start
      setup_ran = true;
      Serial.print("time elapsed for data line start "); Serial.print(elapsed_time / 1000);
      Serial.print(" milliseconds");
      Serial.println();
    }

    //trigger the pulse at the trigger time
    if ((elapsed_time >= pw_trigger_time) && !pulse_start) {
      trigger_pulse();
      test_col++;
      trigger_pulse();
      test_col--;
      pulse_start = true;
      Serial.print("time elapsed for gate line start "); Serial.print(elapsed_time / 1000);
      Serial.print(" milliseconds");
      Serial.println();
    }

    //trigger hold change after hold time
    if ((elapsed_time >= hold_trigger_time) && !hold_ran) {
      trigger_hold(final_state1);
      test_col++;
      trigger_hold(final_state2);
      test_col--;
      hold_ran = true;
      Serial.print("time elapsed for data line end "); Serial.print(elapsed_time / 1000);
      Serial.print(" milliseconds");
      Serial.println();
    }

    //end pulse after pulse trigger time
    if ((elapsed_time >= pw_end_time) && !pulse_end) {
      end_pulse();
      test_col++;
      end_pulse();
      test_col--;
      pulse_end = true;
      Serial.print("time elapsed for gate line end "); Serial.print(elapsed_time / 1000);
      Serial.print(" milliseconds");
      Serial.println();
    }

  }

  //delay longer on the backend if the final state is 0
  // if (final_state) {
  //   delay(200);
  // } else {
  //   delay(600);
  // }

  //all triggers done
  digitalWrite(led, HIGH);
}
