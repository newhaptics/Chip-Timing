int gauge[] = {0, 1, 2, 3, 4, 5}; // set pins according to appropriate ANALOG IN pins on Arduino

float source_pressure = 0;
byte source_pressure_input = 5;
float manifold_pressure = 0;
byte manifold_pressure_input = 4;

boolean test_order[] = {1, 0};

//int num_gauges = sizeof(gauge) / sizeof(gauge[0]);

int led = 9;
byte hi_pressure = 15;

byte SER_Pin   = 7;  // data
byte SRCLK_Pin = 6;  // clock
byte RCLK_Pin  = 5;  // latch
byte CLR_Pin   = 13;  // clr (clears register - active LOW)
byte G_Pin     = 12;  // output gate (output active when G=LOW)
byte G_LED_Pin = 11;  // output gate (output active when G=LOW

#define valves_per_manifold 8
byte valve[8]; // set of 8 bytes stored for valve manifolds

byte source[] = {1, 7};

// Define the outputs of the manifolds
byte row[][2] = {{1, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}};
byte col[][2] = {{2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}};

byte brightness = 240;  // brightness of LEDs, 0 = fully ON :: 255 = fully off

byte regulator = 3;


//byte valve_states = 0;
//byte slider_state = 0;
//int val;
//unsigned int data;

unsigned long current_time;
unsigned long start_time;

byte num_manifolds = 4;
//byte num_gauges = 0;
//boolean regulator_control = 0;

float lin_slope[] = {0, 0, 0, 0, 1 / (.8 * 5 / 100) * 5 / 1023, 1 / .0667 * 5 / 1023};
float lin_yint[] = {0, 0, 0, 0, -.5 / (.8 * 5 / 100), -.5 / .0667};

// string variables
char chip_version[] = "M2A";
byte chip_number = 1;
byte test_row = 0;
byte test_col = 0;

signed long pulse_width = 100000;
signed long setup_time = 200000;
signed long hold_time = 100000;

signed long diff = 10000;
boolean final_state = 1;
byte fail_count;

void reset(boolean serial_out = 0);
void run_pw_test(boolean serial_out = 0);

void setup() {
  TCCR1B = TCCR1B & B11111000 | B00000101; // for PWM frequency of 30.64 Hz
  pinMode(led, OUTPUT);

  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  pinMode(CLR_Pin, OUTPUT);
  pinMode(G_Pin, OUTPUT);
  pinMode(G_LED_Pin, OUTPUT);

  pinMode(regulator, OUTPUT);
  Serial.begin(500000);

  digitalWrite(CLR_Pin, HIGH);

  digitalWrite(G_Pin, LOW);
  analogWrite(G_LED_Pin, 250);

  set_all_valves(LOW);


  initialize();

}

void loop() {

  timing_tests();

}

void timing_tests() {
  long min_setup;
  long min_hold;
  long min_pw;




  Serial.println();
  Serial.print(F("What row are you testing?  "));
  while (Serial.available() == 0);
  test_row = Serial.parseInt();
  serialFlush();
  Serial.println(test_row);

  Serial.print(F("What column are you testing?  "));
  while (Serial.available() == 0);
  test_col = Serial.parseInt();
  serialFlush();
  Serial.println(test_col);


  for (int k = 0; k <= 1; k++) {
    final_state = test_order[k];

    if (final_state == 1) {
      pulse_width = 35000;
      setup_time = 50000;
      hold_time = 80000;
    }
    else {
      pulse_width = 100000;
      setup_time = 100000;
      hold_time = 200000;
    }

    Serial.println();
    Serial.println(F("Now testing for Setup time"));
    Serial.print(F("Starting Setup Time: ")); Serial.println(setup_time);
    Serial.print(F("Starting Hold Time: ")); Serial.println(hold_time);
    Serial.print(F("Pulse Width: ")); Serial.println(pulse_width);
    Serial.println(F("Press enter to reset the fluidic element."));
    enter_to_advance();
    reset();

    //find the minimum setup time for given pulse width
    min_setup = time_test('s');

    cycle();
    Serial.println();
    Serial.println(F("Now testing for Hold time"));
    Serial.print(F("Minimum Setup Time: ")); Serial.println(min_setup);
    Serial.print(F("Starting Hold Time: ")); Serial.println(hold_time);
    Serial.print(F("Pulse Width: ")); Serial.println(pulse_width);
    Serial.println(F("Press enter to reset the fluidic element."));
    enter_to_advance();
    reset();

    //use that setup time to find minimum hold time
    setup_time = min_setup;
    min_hold = time_test('h');


    cycle();
    //finally check if the pulse width can go any lower
    Serial.println();
    Serial.println(F("Now testing for lowest pulse width"));
    Serial.print(F("Minimum Setup Time: ")); Serial.println(min_setup);
    Serial.print(F("Minimum Hold Time: ")); Serial.println(min_hold);
    Serial.print(F("Starting Pulse Width: ")); Serial.println(pulse_width);
    Serial.println(F("Press enter to reset the fluidic element."));
    enter_to_advance();
    reset();

    //find the smallest pulse width
    hold_time = min_hold;
    min_pw = time_test('p');


    Serial.println(F("Go to AmScope and click Record. Select the following options:"));
    Serial.println(F(" -------------------- "));
    Serial.println(F("Video Format: .avi"));
    Serial.println("Filename: ");
    Serial.println();
    Serial.print(chip_version);
    Serial.print("_"); Serial.print(chip_number);
    Serial.print("_r"); Serial.print(test_row);
    Serial.print("_c"); Serial.print(test_col);
    Serial.print("_in"); Serial.print(final_state);
    Serial.print("_s"); Serial.print(min_setup);
    Serial.print("_h"); Serial.print(min_hold);
    Serial.print("_p"); Serial.print(min_pw);
    Serial.println();

    Serial.println(F("Encoder: none"));
    Serial.println(F("Quality: doesn't matter"));
    Serial.println(F("Time Limit: 2 seconds"));
    Serial.println(F("Frame Rate: 1/16 (slider all the way to the left)"));
    Serial.println(F(" -------------------- "));
    Serial.println();
    Serial.println(F("Press enter to cycle through tests to make sure it works."));

    enter_to_advance();

    cycle();


    boolean test_done = 0;
    while (test_done == 0) {


      Serial.println(F(" --------- "));
      Serial.println(F("Once you click 'enter' in the Serial Monitor, a countdown from 5 will begin. Press 'Finish' in AmScope on 0."));

      enter_to_advance();



      signed long pw;
      signed long h;
      signed long s;
      for (int i = 5; i >= 0; i--) {
        Serial.println(i);
        s = setup_time + i * 10000 / 5;
        h = hold_time + i * 10000 / 5;
        pw = pulse_width + i * 10000 / 5;
        timing_trigger(s, pw, h);
        if (i != 0) reset(1);
      }

      delay(2000);
      reset(1);
      serialFlush();

      Serial.println();
      Serial.println(F("Recording done. Check recorded video."));
      Serial.println(F("   'd' if capture was a success."));
      Serial.println(F("   'f' if circuit didn't work. "));
      Serial.println(F("   Enter to repeat if video recording/capture failed. "));

      while (Serial.available() == 0);
      char response = Serial.read();
      serialFlush();
      if (response == 'd') {
        test_done = 1;
      }
      else if (response == 'f') {
        pulse_width = pulse_width + 1000;
      }
      else {
        reset();

      }


    }

  }
}


//finds the smallest timing value for given parameter
//'s' returns the smallest working setup time
//'h' returns the smallest working hold time
//'p' returns the smallest working pulse width
signed long time_test(char test) {
  long last_working_ts;
  long testing_time;
  fail_count = 0;

  //decide which value is being tested
  switch (test) {
    case 's':
      diff = 10000;
      testing_time = setup_time;
      break;
    case 'h':
      testing_time = hold_time;
      diff = 10000;
      break;
    case 'p':
      testing_time = pulse_width;
      diff = 1000;
      break;
  }

  last_working_ts = testing_time;

  while (diff > 100) {

        Serial.print(F("Starting Setup Time: ")); Serial.println(setup_time);
        Serial.print(F("Starting Hold Time: ")); Serial.println(hold_time);
        Serial.print(F("Pulse Width: ")); Serial.println(pulse_width);

    Serial.println();
    Serial.print(test); Serial.print(F(" Time: ")); Serial.print(testing_time);
    Serial.print(F(" -- Step size: ")); Serial.println(diff);
    Serial.println(F("    --> Enter to Start Test"));

    enter_to_advance();

    //test appropriate variable
    switch (test) {
      case 's':
        timing_trigger(testing_time, pulse_width, hold_time);
        break;
      case 'h':
        timing_trigger(setup_time, pulse_width, testing_time);
        break;
      case 'p':
        timing_trigger(setup_time, testing_time, hold_time);
        break;
    }

    Serial.println();
    Serial.println(F("Test done."));
    Serial.println(F("   -> Press enter if success."));
    Serial.println(F("   -> Type 'f' if failure."));
    Serial.println(F("   -> Type 'r' to repeat."));
    Serial.println(F("   -> Type 's' to set the time."));
    Serial.println(F("   -> Type 'd' to set diff."));
    Serial.println(F("   -> Type 'c' to run it as a cycle"));

    while (Serial.available() == 0);
    char response = Serial.read();
    serialFlush();

    //if test failed backtrack and change increment
    if (response == 'f') {
      Serial.println(F("Test Failure"));
      //On first test fail move up the time and increment the fail counter
      if (fail_count == 0) {

        testing_time = testing_time + diff * 1.3;
        diff = diff / 10;
        fail_count++;
      }
      else {
        testing_time = testing_time + diff * 5;
        fail_count++;
      }
    }

    //repeat the test
    else if (response == 'r') {
      Serial.println("Repeat");
      // do nothing, just run again
    }

    //run the test with cycling
    else if (response == 'c') {
      signed long temp;
      Serial.println("Cycling");
      //change the approprate variable to the proper time
      //test appropriate variable
      switch (test) {
        case 's':
          temp = setup_time;
          setup_time = testing_time;
          break;
        case 'h':
          temp = hold_time;
          hold_time = testing_time;
          break;
        case 'p':
          temp = pulse_width;
          pulse_width = testing_time;
          break;
      }
      cycle();
      switch (test) {
        case 's':
          setup_time = temp;
          break;
        case 'h':
          hold_time = temp;
          break;
        case 'p':
          pulse_width = temp;
          break;
      }
    }

    //custom timing
    else if (response == 's') {
      Serial.println(F("Type new time (0 - 100000)"));
      serialFlush();
      while (Serial.available() == 0);
      long new_ts = Serial.parseInt();
      if (new_ts > 0 && new_ts < 200000) {
        testing_time = (unsigned long) new_ts;
      }
      else {
        Serial.println(F("Time unchanged. Try again."));
      }
      serialFlush();
      // do nothing, just run again
    }

    //set a custom test step size
    else if (response == 'd') {
      Serial.println(F("Type new diff (10-10000)"));
      serialFlush();
      while (Serial.available() == 0);
      long new_diff = Serial.parseInt();
      if (new_diff >= 10 && new_diff <= 10000) {
        diff = (unsigned long) new_diff;
      }
      else {
        Serial.println(F("Diff unchanged. Try again."));
      }
      serialFlush();
      // do nothing, just run again
    }

    //if test succeeds subtract the test step size and repeat test
    else {
      Serial.println("Success!");
      last_working_ts = testing_time;
      testing_time = testing_time - diff;
      fail_count = 0;
    }

    reset(1);

  }

  return last_working_ts;
}

void enter_to_advance() {
  serialFlush();
  while (Serial.read() != 10);
  serialFlush();
}


void reset(boolean serial_out) {
  if (!serial_out) Serial.println(F("Resetting..."));
  set_valve(row[test_row], 0);
  set_valve(col[test_col], !final_state);
  delay(250);
  set_valve(row[test_row], 1);
  delay(250);
  //  set_valve(col[test_col],final_state);
  if (!serial_out) Serial.println(F("Reset complete."));
}

void cycle() {
  signed long pw;
  signed long h;
  signed long s;
  for (int i = 5; i >= 0; i--) {

    s = setup_time + i * 10000 / 5;
    h = hold_time + i * 10000 / 5;
    pw = pulse_width + i * 10000 / 5;
    timing_trigger(s, pw, h);
    reset(1);
  }

}





//Sets up a test with a specified setup time, hold time, and pulse width
//ts is setup time; tpw is the pulse width time; th is the hold time
void timing_trigger(signed long ts, signed long tpw, signed long th) {
  //boolean values to determine if each pulse has ran
  bool setup_ran = false;
  bool pulse_start = false;
  bool pulse_end = false;
  bool hold_ran = false;

  //determine trigger times relative to the negative edge of pulse
  unsigned long elapsed_time;
  unsigned long pw_trigger_time = 500000;
  unsigned long setup_trigger_time = pw_trigger_time - ts;
  unsigned long hold_trigger_time = pw_trigger_time + th;
  unsigned long pw_end_time = pw_trigger_time + tpw;

  //  Serial.println(pw_trigger_time);
  //  Serial.println(setup_trigger_time);
  //  Serial.println(hold_trigger_time);
  //  Serial.println(pw_end_time);
  delay(250);

  digitalWrite(led, LOW);
  start_time = micros();

  //wait for time to elapse and trigger elements on the proper times
  while (!(setup_ran && pulse_start && pulse_end && hold_ran)) {
    elapsed_time = micros() - start_time;


    //trigger setup change relative to pulse width trigger
    if ((elapsed_time >= setup_trigger_time) && !setup_ran) {
      trigger_setup(final_state);
      setup_ran = true;
    }

    //trigger the pulse at the trigger time
    if ((elapsed_time >= pw_trigger_time) && !pulse_start) {
      trigger_pulse();
      pulse_start = true;
    }

    //trigger hold change after hold time
    if ((elapsed_time >= hold_trigger_time) && !hold_ran) {
      trigger_hold(final_state);
      hold_ran = true;
    }

    //end pulse after pulse trigger time
    if ((elapsed_time >= pw_end_time) && !pulse_end) {
      end_pulse();
      pulse_end = true;
    }

  }

  //all triggers done
  digitalWrite(led, HIGH);

  delay(250);
}

void run_pw_test(boolean serial_out) {
  //  digitalWrite(led,HIGH);
  set_valve(col[test_col], final_state);
  delay(250);
  //  set_valve(col[test_col],final_state);

  set_valve(row[test_row], 0);
  digitalWrite(led, LOW);
  start_time = micros();
  while (micros() - start_time <= pulse_width);
  set_valve(row[test_row], 1);
  digitalWrite(led, HIGH);
  delay(250);
  //  digitalWrite(led,LOW);
  if (!serial_out) {
    Serial.print("Pulse width: ");
    Serial.println(pulse_width);
  }
}


// --------------- initialize --------------- //
// Set up everything at the start of the program
void initialize() {

  Serial.print(F("Chip Version: ")); Serial.println(chip_version);

  Serial.println("Enter chip number (Ex. 1):");
  while (Serial.available() == 0);
  chip_number = Serial.parseInt();
  serialFlush();
  Serial.print(F("Chip Number: ")); Serial.println(chip_number);

  Serial.println();
  Serial.println(F("Turn on 24 V supply to solenoid valves."));
  set_valve(source, 1);
  Serial.println(F("Source is turned ON."));
  Serial.print(F("After pressing 'enter', increase Source pressure to ")); Serial.print(hi_pressure);
  Serial.println(F(" psi using precision regulator."));
  while (Serial.read() != 10); //wait for user to press enter.
  while (source_pressure < 15) {
    source_pressure = analogRead(source_pressure_input) * lin_slope[source_pressure_input] + lin_yint[source_pressure_input];
    manifold_pressure = analogRead(manifold_pressure_input) * lin_slope[manifold_pressure_input] + lin_yint[manifold_pressure_input];
    Serial.print("Manifold Pressure: "); Serial.print(manifold_pressure); Serial.print(" psi --- ");
    Serial.print("Source Pressure: "); Serial.print(source_pressure); Serial.println(" psi  <-- Increase Source Pressure to 16 psi");
  }
  Serial.println();
  Serial.println(F("Boot up AmScope & click MU043M under 'Camera List'. "));
  Serial.println(F("Under 'Exposure & Gain', turn OFF 'Auto Explosure' & set:"));
  Serial.println(F("    Exposure Time = 0.5 ms"));
  Serial.println(F("    Gain = 100%"));
  Serial.println(F("Move XY-Positioning stage to desired fluidic element and press enter."));
  while (Serial.read() != 10); //wait for user to press enter.
  serialFlush();

  //  delay(1000);

}

// --------------- set_valve --------------- //
// Takes as input valve pair indicated manifold and valve in that manifold
void set_valve(byte valve_pair[], boolean val) {
  bitWrite(valve[valve_pair[0]], 7 - valve_pair[1], val);
  update_valves();
}

// --------------- update_valves --------------- //
// Pushes byte array "valve" to shift registers to set the state of the electronic valves
void  update_valves() {
  digitalWrite(RCLK_Pin, LOW);

  for (int i = num_manifolds - 1; i >= 0; i--) {
    shiftOut(SER_Pin, SRCLK_Pin, LSBFIRST, valve[i]);
  }
  digitalWrite(RCLK_Pin, HIGH);

}

// --------------- set_all_valves --------------- //
// Choose to set state of all valves either HIGH or LOWs
void set_all_valves(boolean state) {
  for (int i = 0; i < num_manifolds; i++) {
    if (state) valve[i] = 255;
    else valve[i] = 0;
  }
}

// --------------- serialFlush --------------- //
// Read all available Serial data from com port.
void serialFlush() {
  while (Serial.available() > 0) Serial.read();
}


// ---------------- latch and data functions -----------//
//turns on data of the current cell
void data_on() {
  set_valve(col[test_col], 1);
}

//turns off data of the current cell
void data_off() {
  set_valve(col[test_col], 0);
}

//turns on latch of current cell
void latch_on() {
  set_valve(row[test_row], 1);
}

//turns off latch of current cell
void latch_off() {
  set_valve(row[test_row], 0);
}

// ----------- timing functions ---------//

//tests the setup time by changing data to the final state
void trigger_setup(bool final_state) {
  if (final_state) {
    data_on();
  } else {
    data_off();
  }
  //  Serial.println(F("setup trigger!"));
}


//starts the edge of the pulse
void trigger_pulse() {
  latch_off();
  //  Serial.println(F("pulse trigger!"));
}

//tests the hold time by changing data to the opposite of final state
void trigger_hold(bool final_state) {
  if (final_state) {
    data_off();
  } else {
    data_on();
  }
  //  Serial.println(F("hold trigger!"));
}

//ends the pulse
void end_pulse() {
  latch_on();
  //  Serial.println(F("end pulse!"));
}




























//might be irrelevant
void pulse_width_testing() {
  long last_working_pw;

  Serial.println();
  Serial.print(F("What row are you testing?  "));
  while (Serial.available() == 0);
  test_row = Serial.parseInt();
  serialFlush();
  Serial.println(test_row);

  Serial.print(F("What column are you testing?  "));
  while (Serial.available() == 0);
  test_col = Serial.parseInt();
  serialFlush();
  Serial.println(test_col);


  for (int k = 0; k <= 1; k++) {
    final_state = test_order[k];
    Serial.println(" ---------- ");
    Serial.print(F("Desired final state: "));
    Serial.println(final_state);

    if (final_state == 1) pulse_width = 100000;
    else pulse_width = 150000;

    diff = 10000;
    Serial.println();
    Serial.println(F("Press enter to reset the fluidic element."));
    enter_to_advance();
    reset();


    fail_count = 0;
    while (diff > 100) {

      Serial.println();
      Serial.print(F("Pulse Width: ")); Serial.print(pulse_width);
      Serial.print(F(" -- Step size: ")); Serial.println(diff);
      Serial.println(F("    --> Enter to Start Test"));

      enter_to_advance();

      run_pw_test();

      Serial.println();
      Serial.println(F("Test done."));
      Serial.println(F("   -> Press enter if success."));
      Serial.println(F("   -> Type 'f' if failure."));
      Serial.println(F("   -> Type 'r' to repeat."));
      Serial.println(F("   -> Type 'p' to set pulse width."));
      Serial.println(F("   -> Type 'd' to set diff."));

      while (Serial.available() == 0);
      char response = Serial.read();
      serialFlush();
      if (response == 'f') {
        Serial.println(F("Test Failure"));
        if (fail_count == 0) {

          pulse_width = pulse_width + diff * 1.3;
          diff = diff / 10;
          fail_count++;
        }
        else {
          pulse_width = pulse_width + diff * 5;
          fail_count++;
        }
      }
      else if (response == 'r') {
        Serial.println("Repeat");
        // do nothing, just run again
      }
      else if (response == 'p') {
        Serial.println(F("Type new pulse width (0 - 100000)"));
        serialFlush();
        while (Serial.available() == 0);
        long new_pw = Serial.parseInt();
        if (new_pw > 0 && new_pw < 200000) {
          pulse_width = (unsigned long) new_pw;
        }
        else {
          Serial.println(F("Pulse width unchanged. Try again."));
        }
        serialFlush();
        // do nothing, just run again
      }
      else if (response == 'd') {
        Serial.println(F("Type new diff (10-10000)"));
        serialFlush();
        while (Serial.available() == 0);
        long new_diff = Serial.parseInt();
        if (new_diff >= 10 && new_diff <= 10000) {
          diff = (unsigned long) new_diff;
        }
        else {
          Serial.println(F("Diff unchanged. Try again."));
        }
        serialFlush();
        // do nothing, just run again
      }
      else {
        Serial.println("Success!");
        last_working_pw = pulse_width;
        pulse_width = pulse_width - diff;
        fail_count = 0;
      }

      reset(1);

    }


    Serial.println(F("Go to AmScope and click Record. Select the following options:"));
    Serial.println(F(" -------------------- "));
    Serial.println(F("Video Format: .avi"));
    Serial.println("Filename: ");
    Serial.println();
    Serial.print(chip_version);
    Serial.print("_"); Serial.print(chip_number);
    Serial.print("_r"); Serial.print(test_row);
    Serial.print("_c"); Serial.print(test_col);
    Serial.print("_in"); Serial.print(final_state);
    Serial.print("_pw"); Serial.println(last_working_pw);
    Serial.println();

    Serial.println(F("Encoder: none"));
    Serial.println(F("Quality: doesn't matter"));
    Serial.println(F("Time Limit: 2 seconds"));
    Serial.println(F("Frame Rate: 1/16 (slider all the way to the left)"));
    Serial.println(F(" -------------------- "));
    Serial.println();
    Serial.println(F("Press enter to cycle through tests to make sure it works."));

    enter_to_advance();

    // cycle through a couple tests at higher pulse widths, working down to final test pulse width
    cycle();

    boolean test_done = 0;
    while (test_done == 0) {


      Serial.println(F(" --------- "));
      Serial.println(F("Once you click 'enter' in the Serial Monitor, a countdown from 5 will begin. Press 'Finish' in AmScope on 0."));

      enter_to_advance();

      for (int i = 5; i >= 0; i--) {
        Serial.println(i);
        pulse_width = last_working_pw + i * 100000 / 5;
        run_pw_test(1);
        if (i != 0) reset(1);
      }
      delay(2000);
      reset(1);
      serialFlush();

      Serial.println();
      Serial.println(F("Recording done. Check recorded video."));
      Serial.println(F("   'd' if capture was a success."));
      Serial.println(F("   'f' if circuit didn't work. "));
      Serial.println(F("   Enter to repeat if video recording/capture failed. "));

      while (Serial.available() == 0);
      char response = Serial.read();
      serialFlush();
      if (response == 'd') {
        test_done = 1;
      }
      else if (response == 'f') {
        pulse_width = pulse_width + 1000;
      }
      else {
        reset();

      }


    }

  }

  serialFlush();
}
