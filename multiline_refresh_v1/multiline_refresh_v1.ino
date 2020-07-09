#include <Chip_Timing_library.h>

bool test_order[] = {1, 0};

//elements to be tested
char elemTests[4][2] = {{0,0},{1,0},{2,0},{3,0},{4,0}};

//final states
char final_states[4] = {0,0,0,0};

//timing for each element
char elemTiming[4][3] = {{10,10,19}, {10,10,19}, {10,10,19}, {10,10,19}};

// string variables
char chip_version[] = "M3AR";
byte chip_number = 1;

signed long pulse_width = 100000;
signed long setup_time = 200000;
signed long hold_time = 100000;

int test_row = elemTests[0][0];
int test_col = elemTests[0][1];


//create a cellTest object
cellTest FC(test_row,test_col,final_state);

void setup() {
  TCCR1B = TCCR1B & B11111000 | B00000101; // for PWM frequency of 30.64 Hz
  Serial.begin(57600);

  FC.cell_setup();
  
  
  FC.set_all_valves(LOW);


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
  test_row--;
  Serial.print(F("What column are you testing?  "));
  while (Serial.available() == 0);
  test_col = Serial.parseInt();
  serialFlush();
  Serial.println(test_col);
  test_col--;
  //cycle through testing 1 to 0 and 0 to 1
  for (long k = 0; k <= 1; k++) {
    final_state = test_order[k];
    FC.testCell(test_row, test_col, final_state);
    if (final_state == 1) {
      //        pulse_width = 50000 - i * 5000;
      pulse_width = 30000;// - i * 20000;
      setup_time = 20000;
      hold_time = pulse_width + (5000 * long(1 + test_row)) + (5000 * long(1 + test_col));
    }
    else {
      pulse_width = 30000;// + i * 20000
      setup_time = 20000;
      hold_time = pulse_width + 5000 * long(1 + test_row) + (5000 * long(1 + test_col));
    }

    Serial.print(F("Final state is: ")); Serial.print(final_state); Serial.println();
    Serial.print(F("Testing with pulse width: ")); Serial.print(pulse_width); Serial.println();
    enter_to_advance();

    cycle();

    Serial.println();
    Serial.println(F("Now testing for Setup time"));


    //find the minimum setup time for given pulse width
    min_setup = time_test('s');

    Serial.println();
    Serial.println(F("Now testing for Hold time"));

    //use that setup time to find minimum hold time
    setup_time = min_setup;
    min_pw = pulse_width;
    min_hold = time_test('h');


    //finally check if the pulse width can go any lower
    //    Serial.println();
    //    Serial.println(F("Now testing for lowest pulse width"));
    //
    //    //find the smallest pulse width
    //    hold_time = min_hold;
    //    min_pw = time_test('p');


    Serial.println(F("Go to AmScope and click Record. Select the following options:"));
    Serial.println(F(" -------------------- "));
    Serial.println(F("Video Format: .avi"));
    Serial.println("Filename: ");
    Serial.println();
    Serial.print(chip_version);
    Serial.print("_"); Serial.print(chip_number);
    Serial.print("_r"); Serial.print(test_row + 1);
    Serial.print("_c"); Serial.print(test_col + 1);
    Serial.print("_in"); Serial.print(final_state);
    Serial.print("_s"); Serial.print(min_setup / 1000);
    Serial.print("_h"); Serial.print(min_hold / 1000);
    Serial.print("_p"); Serial.print(min_pw / 1000); Serial.print("_");
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
      Serial.println(F("Once you click 'enter' in the Serial Monitor, a countdown from 10 will begin. Press 'Finish' in AmScope on 0."));

      enter_to_advance();



      signed long pw;
      signed long h;
      signed long s;
      for (int i = 5; i >= 0; i--) {
        Serial.println(i);
        s = setup_time + i * 1000;
        h = hold_time + i * 1000;
        pw = pulse_width;
        FC.timing_trigger(s, pw, h);
        if (i != 0) FC.reset(1);
      }

      delay(2000);
      FC.reset(1);
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
        FC.reset(0);

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

  //setup offset multiplier
  long setup_offset = 3;

  //decide which value is being tested
  switch (test) {
    case 's':
      diff = 10000;
      testing_time = setup_time;
      break;
    case 'h':
      diff = 10000;
      testing_time = hold_time;
      break;
    case 'p':
      diff = 1000;
      testing_time = pulse_width + 3 * diff;

      break;
  }

  last_working_ts = testing_time;

  Serial.print(F("Starting Setup Time: ")); Serial.println(setup_time);
  Serial.print(F("Starting Hold Time: ")); Serial.println(hold_time);
  Serial.print(F("Pulse Width: ")); Serial.println(pulse_width);
  Serial.print(F("Press enter on failure"));
  Serial.println();

  while (fail_count < 2) {

    //loop through tests constantly until enter is pressed
    while (Serial.read() != 10) {
      Serial.print(test); Serial.print(F(" Time: ")); Serial.print(testing_time);
      Serial.print(F(" -- Step size: ")); Serial.println(diff);
      Serial.println();

      testing_time = testing_time - diff;



      //test appropriate variable
      switch (test) {
        case 's':
          FC.timing_trigger(testing_time, pulse_width, hold_time);
          break;
        case 'h':
          FC.timing_trigger(setup_time + setup_offset * 1000, pulse_width, testing_time);
          break;
        case 'p':
          FC.timing_trigger(setup_time, testing_time, hold_time);
          break;
      }

      if (setup_offset > 0) {
        testing_time += diff;
        setup_offset--;
      }

      FC.reset(1);
    }

    //On first test fail move up the time and increment the fail counter
    if (fail_count < 1) {

      testing_time = testing_time + diff * 2;
      diff = diff / 10;
      fail_count++;
    }
    else {
      Serial.println("Success!");

      //offset test value by 3 mil if final state is 1 and by 20 mil if final state is 0
      if (final_state) {
        last_working_ts = testing_time + 3 * diff;
      } else {
        last_working_ts = testing_time + 3 * diff;
      }

      fail_count++;
    }
    serialFlush();

  }

  return last_working_ts;
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

  FC.start_chip();
  
  Serial.println(F("Move XY-Positioning stage to desired fluidic element and press enter."));
  while (Serial.read() != 10); //wait for user to press enter.
  serialFlush();

  //  delay(1000);

}

// --------------- serialFlush --------------- //
// Read all available Serial data from com port.
void serialFlush() {
  while (Serial.available() > 0) Serial.read();
}

void enter_to_advance() {
  serialFlush();
  while (Serial.read() != 10);
  serialFlush();
}

void cycle(boolean serial_out) {
  signed long pw;
  signed long h;
  signed long s;
  if (!serial_out) Serial.println("Cycling...");
  for (int i = 5; i >= 0; i--) {

    s = setup_time + i * 1000;
    h = hold_time + i * 1000;
    pw = pulse_width;
    FC.timing_trigger(s, pw, h);
    FC.reset(1);
  }

}
