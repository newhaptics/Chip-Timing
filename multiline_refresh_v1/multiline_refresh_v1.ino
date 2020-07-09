#include <Chip_Timing_library.h>

//response to questions
char response = 'n';

//number of elements to be tested
const int numElem = 5;


//elements to be tested
const int elemTests[numElem][2] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}};

//final states
const int final_states[numElem] = {1, 1, 1, 1};

//timing for each element
const int elemTiming[numElem][3] = {{100, 100, 40}, {100, 100, 40}, {100, 100, 40}, {100, 100, 40}};

// string variables
char chip_version[] = "M3AR";
byte chip_number = 1;

signed long pulse_width = 100000;
signed long setup_time = 200000;
signed long hold_time = 100000;

int test_row = elemTests[0][0];
int test_col = elemTests[0][1];

int final_state;

//create a cellTest object
cellTest FC(test_row, test_col, final_state);

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

}

void loop() {

  stream_test();

}

void stream_test() {



  //display the elements being tested
  Serial.println();
  Serial.print(F("Now testing the following elements "));
  for (int i = 0; i < numElem; i++) {
    Serial.print("Element "); Serial.print(i);
    Serial.print("in row "); Serial.print(elemTests[i][0]);
    Serial.print("column "); Serial.print(elemTests[i][1]);
    Serial.print("with a final state of "); Serial.print(final_states[i]); Serial.println();
  }

  //perform the test
  multiline_test();


  Serial.print("did the test succeed? y/n");
  while (Serial.available() == 0);
  response = Serial.parseInt();
  serialFlush();

  if (response == 'y') {

    test_row = 0;
    test_col = 0;
    final_state = 0;
    setup_time = 0;
    hold_time = 0;
    pulse_width = 0;

    Serial.println(F("Go to AmScope and click Record. Select the following options:"));
    Serial.println(F(" -------------------- "));
    Serial.println(F("Video Format: .avi"));
    Serial.println("Filename: ");
    Serial.println();
    Serial.print("S"); Serial.print(numElem);
    Serial.print("_");
    Serial.print(chip_version);
    Serial.print("_"); Serial.print(chip_number);

    //generate row, column, final_state... pairs
    for (int i = 0; i < numElem; i++) {
      Serial.print("_E"); Serial.print(i);

      test_row = elemTests[i][0];
      test_col = elemTests[i][1];
      final_state = final_states[i];
      setup_time = elemTiming[i][0];
      hold_time = elemTiming[i][1];
      pulse_width = elemTiming[i][2];

      Serial.print("_r"); Serial.print(test_row + 1);
      Serial.print("_c"); Serial.print(test_col + 1);
      Serial.print("_in"); Serial.print(final_state);
      Serial.print("_s"); Serial.print(setup_time);
      Serial.print("_h"); Serial.print(hold_time);
      Serial.print("_p"); Serial.print(pulse_width); Serial.print("_");
      Serial.println();

    }

    Serial.println(F("Encoder: none"));
    Serial.println(F("Quality: doesn't matter"));
    Serial.println(F("Time Limit: 2 seconds"));
    Serial.println(F("Frame Rate: 1/16 (slider all the way to the left)"));
    Serial.println(F(" -------------------- "));
    Serial.println();
    Serial.println(F("Press enter to begin test."));

    enter_to_advance();


    boolean test_done = 0;
    while (test_done == 0) {


      Serial.println(F(" --------- "));
      Serial.println(F("Once you click 'enter' in the Serial Monitor, a countdown from 5 will begin. Press 'Finish' in AmScope on 0."));

      enter_to_advance();



      for (int i = 5; i >= 0; i--) {
        Serial.println(i);
        multiline_test();
      }

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

    Serial.println("Oopsie Woopsie");
  }



}

//multiline test to take the three arrays and use them to activate fluidic elements

void multiline_test() {
  //notify user all lines are being reset
  Serial.print("Resetting all the lines"); Serial.println();

  //loop through each element of the tests
  for (int i = 0; i < numElem; i++) {
    //assign ts, th, and tpw for the current test as well as current row and final state
    test_row = elemTests[i][0];
    test_col = elemTests[i][1];
    final_state = final_states[i];

    Serial.print("Element "); Serial.print(i);
    Serial.print("in row "); Serial.print(elemTests[i][0]);
    Serial.print("column "); Serial.print(elemTests[i][1]);
    Serial.print("to a state of "); Serial.print(!final_states[i]); Serial.println();


    //assign the test cell to the cellTest object
    FC.testCell(test_row, test_col, final_state);

    //reset the lines
    FC.reset(0);

  }

  Serial.print("press enter to begin test"); Serial.println();

  enter_to_advance();

  //loop through each element of the tests
  for (int i = 0; i < numElem; i++) {
    //assign ts, th, and tpw for the current test as well as current row and final state
    test_row = elemTests[i][0];
    test_col = elemTests[i][1];
    final_state = final_states[i];
    setup_time = elemTiming[i][0];
    hold_time = elemTiming[i][1];
    pulse_width = elemTiming[i][2];

    //assign the test cell to the cellTest object
    FC.testCell(test_row, test_col, final_state);

    //activate a timing trigger test on that elements
    FC.timing_trigger(setup_time, pulse_width, hold_time);

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
