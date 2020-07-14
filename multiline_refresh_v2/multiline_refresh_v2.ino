#include <Chip_Timing_library.h>

//response to questions
char response = 'n';

//number of elements to be tested
const int numElem = 5;


//elements to be tested
const int elemTests[numElem][2] =
//{{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}};
{{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}};
//{5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0},
//{5, 1}, {6, 1}, {7, 1}, {8, 1}, {9, 1}};

//final states
const int final_states[numElem][2] =
{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}};
//1, 0, 1, 0, 1, 0, 1, 0, 1, 0};

//timing for each element
const signed long elemTiming[numElem][3] =
//{{-20, 70, 60}, {-20, 70, 60}, {-20, 70, 60}, {-20, 70, 60}, {-20, 70, 60},
//{-14, 32, 30}, {-10, 33, 30}, {-10, 34, 30}, {-5, 33, 30}, {-15, 37, 30}};
{{-4, 40, 30}, {-4, 40, 30}, {-4, 40, 30}, {-4, 40, 30}, {-4, 40, 30}};
//{{-14, 32, 30}, {-10, 33, 30}, {-10, 34, 30}, {-5, 33, 30}, {-15, 37, 30},
//{-14, 32, 30}, {-10, 33, 30}, {-10, 34, 30}, {-5, 33, 30}, {-15, 37, 30},
//{-14, 32, 30}, {-10, 33, 30}, {-10, 34, 30}, {-5, 33, 30}, {-15, 37, 30},
//{-14, 32, 30}, {-10, 33, 30}, {-10, 34, 30}, {-5, 33, 30}, {-15, 37, 30}};

// string variables
char chip_version[] = "M3AR";
byte chip_number = 1;

signed long pulse_width = 100000;
signed long setup_time = 200000;
signed long hold_time = 100000;

int test_row = elemTests[0][0];
int test_col = elemTests[0][1];

int final_state[2] = {0, 0};

//create a cellTest object
cellTest FC(test_row, test_col, final_state[0], final_state[1]);

void multiline_reset();
void initialize();
void multiline_test();
void serialFlush();
void enter_to_advance();

void setup() {
  TCCR1B = TCCR1B & B11111000 | B00000101; // for PWM frequency of 30.64 Hz
  Serial.begin(57600);

  FC.cell_setup();


  FC.set_all_valves(LOW);


  initialize();

  randomSeed(analogRead(0));

}

void loop() {

  stream_test();

}

void stream_test() {



  //display the elements being tested
  Serial.println();
  Serial.print(F("Now testing the following elements :"));
  Serial.println();
  for (int i = 0; i < numElem; i++) {
    Serial.print("Element "); Serial.print(i + 1);
    Serial.print(" in row "); Serial.print(elemTests[i][0] + 1);
    Serial.print(" column "); Serial.print(elemTests[i][1] + 1);
    Serial.print(" and column "); Serial.print(elemTests[i][1] + 2);
    Serial.print(" with a final state of "); Serial.print(final_states[i][0]);
    Serial.print(" and "); Serial.print(final_states[i][1]); Serial.println();
  }

  enter_to_advance();

  multiline_reset();

  Serial.print("press enter to begin test"); Serial.println();

  enter_to_advance();

  //perform the test
  multiline_test();


  Serial.print("did the test succeed? y/n");
  while (Serial.available() == 0);
  response = Serial.read();
  serialFlush();

  int trial_num = random(1000) + random(500) + random(20);
  if (response == 'y') {

    test_row = 0;
    test_col = 0;
    final_state[0] = 0;
    final_state[1] = 0;
    setup_time = 0;
    hold_time = 0;
    pulse_width = 0;

    Serial.println(F("Go to AmScope and click Record. Select the following options:"));
    Serial.println(F(" -------------------- "));
    Serial.println(F("Video Format: .avi"));
    Serial.println("Filename: ");
    Serial.print(chip_version);
    Serial.print("_"); Serial.print(chip_number);
    Serial.print("_"); Serial.print("S"); Serial.print(numElem*2);

    test_row = elemTests[numElem/4][0];
    test_col = elemTests[numElem/2][1];
    final_state[0] = final_states[1][0];
    final_state[1] = final_states[1][1];
    setup_time = elemTiming[numElem*2][0];
    hold_time = elemTiming[numElem*4/3][1];
    pulse_width = elemTiming[numElem][2];

    int myrand = test_row + test_col + final_state[0] + final_state[1] + setup_time + hold_time + pulse_width;
    int theirrand = random(1000) + random(500) + random(50);

      Serial.print("_");
      Serial.print(myrand);
      Serial.print("_");
      Serial.print(theirrand);


    Serial.print("_");
    Serial.print("D");
    Serial.println();
    Serial.println(F("Encoder: none"));
    Serial.println(F("Quality: doesn't matter"));
    Serial.println(F("Time Limit: 2 seconds"));
    Serial.println(F("Frame Rate: 1/16 (slider all the way to the left)"));
    Serial.println(F(" -------------------- "));

    Serial.println(F("Open a txt file and store this test name inside"));
    Serial.println("Testname: ");
    Serial.print(chip_version);
    Serial.print("_"); Serial.print(chip_number);
    Serial.println();
    Serial.print("S"); Serial.print(numElem*2); Serial.print("_");

    //generate row, column, final_state... pairs
    for (int i = 0; i < numElem; i++) {
      Serial.print("_E"); Serial.print(i + 1);

      test_row = elemTests[i][0];
      test_col = elemTests[i][1];
      final_state[0] = final_states[i][0];
      setup_time = elemTiming[i][0];
      hold_time = elemTiming[i][1];
      pulse_width = elemTiming[i][2];

      Serial.print("_r"); Serial.print(test_row + 1);
      Serial.print("_c"); Serial.print(test_col + 1);
      Serial.print("_in"); Serial.print(final_state[0]);
      Serial.print("_s"); Serial.print(setup_time);
      Serial.print("_h"); Serial.print(hold_time);
      Serial.print("_p"); Serial.print(pulse_width);

      Serial.print("_E"); Serial.print((i + 1) + numElem);

      test_row = elemTests[i][0];
      test_col = elemTests[i][1] + 1;
      final_state[1] = final_states[i][1];
      setup_time = elemTiming[i][0];
      hold_time = elemTiming[i][1];
      pulse_width = elemTiming[i][2];

      Serial.print("_r"); Serial.print(test_row + 1);
      Serial.print("_c"); Serial.print(test_col + 1);
      Serial.print("_in"); Serial.print(final_state[1]);
      Serial.print("_s"); Serial.print(setup_time);
      Serial.print("_h"); Serial.print(hold_time);
      Serial.print("_p"); Serial.print(pulse_width);

    }



    Serial.print("_");
    Serial.print("E");
    Serial.println();
    Serial.println(F("Press enter to begin test."));

    enter_to_advance();


    boolean test_done = 0;
    while (test_done == 0) {


      multiline_reset();

      Serial.println(F(" --------- "));
      Serial.println(F("Once you click 'enter' in the Serial Monitor a countdown from 5 will start, Press 'Finish' on 0 in AmScope"));

      enter_to_advance();

      for(int i = 5; i >= 0; i--){
        delay(100);
        Serial.println(i);
        delay(500);
      }

      multiline_test();

      delay(2000);
      serialFlush();

      Serial.println();
      Serial.println(F("Recording done. Check recorded video."));
      Serial.println(F("   'd' if capture was a success."));
      Serial.println(F("   Enter to repeat if video recording/capture failed. "));

      while (Serial.available() == 0);
      response = Serial.read();
      serialFlush();
      if (response == 'd') {
        test_done = 1;
      }


    }


  } else {

    Serial.println();
    Serial.println("Oopsie Woopsie");
  }



}


void multiline_reset() {
  Serial.print("press enter to reset all the lines"); Serial.println();

  enter_to_advance();

  //notify user all lines are being reset
  Serial.print("Resetting all the lines "); Serial.println();

  //loop through each element of the tests
  for (int i = 0; i < numElem; i++) {
    //assign ts, th, and tpw for the current test as well as current row and final state
    test_row = elemTests[i][0];
    test_col = elemTests[i][1];
    final_state[0] = final_states[i][0];
    final_state[1] = final_states[i][1];


    Serial.print("Element "); Serial.print(i + 1);
    Serial.print(" in row "); Serial.print(elemTests[i][0] + 1);
    Serial.print(" column "); Serial.print(elemTests[i][1] + 1);
    Serial.print(" and column "); Serial.print(elemTests[i][1] + 2);
    Serial.print(" with a final state of "); Serial.print(final_states[i][0]);
    Serial.print(" and "); Serial.print(final_states[i][1]); Serial.println();

    //assign the test cell to the cellTest object
    FC.testCell(test_row, test_col, final_state[0], final_state[1]);

    //reset the lines
    FC.reset(0);

  }
}

//multiline test to take the three arrays and use them to activate fluidic elements

void multiline_test() {
  unsigned long elapsed_time;
  unsigned long start_time;
  unsigned long tot_time = 0;

  //start_time = micros();
  //loop through each element of the tests
  for (int i = 0; i < numElem; i++) {
    //assign ts, th, and tpw for the current test as well as current row and final state
    test_row = elemTests[i][0];
    test_col = elemTests[i][1];
    final_state[0] = final_states[i][0];
    final_state[1] = final_states[i][1];
    setup_time = elemTiming[i][0] * 1000;
    hold_time = elemTiming[i][1] * 1000;
    pulse_width = elemTiming[i][2] * 1000;

    //assign the test cell to the cellTest object
    FC.testCell(test_row, test_col, final_state[0], final_state[1]);

     Serial.print("Element "); Serial.print(i + 1);
     Serial.print(" setup "); Serial.print(setup_time / 1000);
     Serial.print(" hold "); Serial.print(hold_time / 1000);
     Serial.print(" pulse width "); Serial.print(pulse_width / 1000); Serial.println();

    //activate a timing trigger test on that elements
    start_time = micros();

    FC.dual_timing_trigger(setup_time, pulse_width, hold_time);

    elapsed_time = micros() - start_time;
    elapsed_time = elapsed_time / 1000;
    tot_time = tot_time + elapsed_time;
    Serial.print("time elapsed for element "); Serial.print(elapsed_time);
    Serial.print(" milliseconds");
    Serial.println();


  }
  Serial.print("time elapsed since test start "); Serial.print(tot_time);
  Serial.print(" milliseconds");
  Serial.println();


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

  Serial.println(F("Move XY-Positioning into multiline view and press enter."));
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
