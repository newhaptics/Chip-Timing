int gauge[] = {0, 1, 2, 3, 4, 5}; // set pins according to appropriate ANALOG IN pins on Arduino

float source_pressure = 0;
byte source_pressure_input = 5;
float manifold_pressure = 0;
byte manifold_pressure_input = 4;

boolean test_order[] = {0, 1};

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

byte source[] = {1,7};

// Define the outputs of the manifolds
byte row[][2] = {{1,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7}};
byte col[][2] = {{2,0},{2,1},{2,2},{2,3},{2,4},{2,5},{2,6},{2,7}};

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

float lin_slope[] = {0, 0, 0, 0, 1/(.8*5/100)*5/1023, 1/.0667*5/1023};
float lin_yint[] = {0, 0, 0, 0, -.5/(.8*5/100), -.5/.0667};

// string variables
char chip_version[] = "M2A";
byte chip_number = 1;
byte test_row = 0;
byte test_col = 0;

unsigned long pulse_width = 100000;
unsigned long setup_time = 100000;
unsigned long diff = 10000;
boolean final_state = 1;
byte fail_count;

void reset(boolean serial_out = 0);
void run_pw_test(boolean serial_out = 0);

void setup() {
  TCCR1B = TCCR1B & B11111000 | B00000101; // for PWM frequency of 30.64 Hz
  pinMode(led,OUTPUT);
  
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
  
  pulse_width_testing();

}

void setup_time_test(){
  long last_working_ts;
  
  Serial.println();
  Serial.print(F("What row are you testing?  "));
  while(Serial.available()==0);
  test_row= Serial.parseInt();
  serialFlush();
  Serial.println(test_row);
  
  Serial.print(F("What column are you testing?  "));
  while(Serial.available()==0);
  test_col= Serial.parseInt();
  serialFlush();
  Serial.println(test_col);

  for (int k = 0; k<=1; k++){ 
    final_state = test_order[k];

    if (final_state == 1) pulse_width = 100000;
    else pulse_width = 150000;
    
    diff = 10000;
    Serial.println();
    Serial.println(F("Press enter to reset the fluidic element."));
    enter_to_advance();
    reset();
  }
}

void pulse_width_testing(){
  long last_working_pw;
  
  Serial.println();
  Serial.print(F("What row are you testing?  "));
  while(Serial.available()==0);
  test_row= Serial.parseInt();
  serialFlush();
  Serial.println(test_row);
  
  Serial.print(F("What column are you testing?  "));
  while(Serial.available()==0);
  test_col= Serial.parseInt();
  serialFlush();
  Serial.println(test_col);
  

  for (int k = 0; k<=1; k++){ 
    final_state = test_order[k];
    Serial.println(" ---------- ");
    Serial.print(F("Desired final state: "));
    Serial.println(final_state);
    
    if (final_state == 1) pulse_width=100000;
    else pulse_width = 150000;
    
    diff = 10000;
    Serial.println();
    Serial.println(F("Press enter to reset the fluidic element."));
    enter_to_advance();
    reset();
    
    fail_count = 0;
    while (diff > 100){

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
      
      while(Serial.available()==0);
      char response = Serial.read();
      serialFlush();
      if (response == 'f') {
        Serial.println(F("Test Failure"));
        if (fail_count == 0) {        
          
          pulse_width = pulse_width + diff*1.3;
          diff = diff/10;
          fail_count++;
        }
        else{
          pulse_width = pulse_width + diff*5;
          fail_count++;
        }
      }
      else if (response == 'r'){
        Serial.println("Repeat");
        // do nothing, just run again
      }
      else if (response == 'p'){
        Serial.println(F("Type new pulse width (0 - 100000)"));
        serialFlush();
        while(Serial.available()==0);
        long new_pw = Serial.parseInt();
        if (new_pw > 0 && new_pw < 200000){
          pulse_width = (unsigned long) new_pw;
        }
        else{
          Serial.println(F("Pulse width unchanged. Try again."));
        }
        serialFlush();
        // do nothing, just run again
      }
      else if (response == 'd'){
        Serial.println(F("Type new diff (10-10000)"));
        serialFlush();
        while(Serial.available()==0);
        long new_diff = Serial.parseInt();
        if (new_diff >= 10 && new_diff <= 10000){
          diff = (unsigned long) new_diff;
        }
        else{
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
    for (int i=5; i>=0; i--){
      
      pulse_width = last_working_pw + i*10000/5;
      run_pw_test();
      reset(1);
    }


    boolean test_done=0;
    while(test_done==0){
      
    
      Serial.println(" --------- ");
      Serial.println(F("Once you click 'enter' in the Serial Monitor, a countdown from 5 will begin. Press 'Finish' in AmScope on 0."));
      
      enter_to_advance();
      
      for (int i=5; i>=0; i--){
        Serial.println(i);
        pulse_width = last_working_pw + i*100000/5;
        run_pw_test(1);
        if (i!=0) reset(1);
      }
      delay(2000);
      reset(1);
      serialFlush();
      
      Serial.println();
      Serial.println(F("Recording done. Check recorded video."));
      Serial.println(F("   'd' if capture was a success."));
      Serial.println(F("   'f' if circuit didn't work. "));
      Serial.println(F("   Enter to repeat if video recording/capture failed. "));

      while(Serial.available()==0);
      char response = Serial.read();
      serialFlush();
      if (response == 'd') {
        test_done = 1;
      }
      else if (response == 'f'){
        pulse_width=pulse_width+1000;
      }
      else {
        reset();
        
      }
      
      
    }
    
  }

  serialFlush();
}

void enter_to_advance(){
  serialFlush();
  while(Serial.read()!=10);
  serialFlush();
}


void reset(boolean serial_out){
  if (!serial_out) Serial.println(F("Resetting..."));
  set_valve(row[test_row],0);
  set_valve(col[test_col],!final_state);
  delay(250);
  set_valve(row[test_row],1);
  delay(250);
  set_valve(col[test_col],final_state);
  if (!serial_out) Serial.println(F("Reset complete."));
}

void run_ts_test(){
  delay(250);
  set_valve(col[test_col],final_state);
  digitalWrite(led, LOW);
  start_time = micros();
  while (micros() - start_time <= setup_time);
  set_valve(row[test_row],0);
  while (micros() - start_time <= pulse_width);
  set_valve(row[test_row],1);
  digitalWrite(led, HIGH);
  delay(250);
}

void run_pw_test(boolean serial_out){
//  digitalWrite(led,HIGH);
  delay(250);
//  set_valve(col[test_col],final_state);  
  
  set_valve(row[test_row],0);
  digitalWrite(led, LOW);
  start_time = micros();
  while (micros() - start_time <= pulse_width);
  set_valve(row[test_row],1);
  digitalWrite(led, HIGH);
  delay(250);
//  digitalWrite(led,LOW);
  if(!serial_out) {
    Serial.print("Pulse width: "); 
    Serial.println(pulse_width);
  }
}


// --------------- initialize --------------- //
// Set up everything at the start of the program
void initialize(){

  Serial.print(F("Chip Version: ")); Serial.println(chip_version);
  
  Serial.println("Enter chip number (Ex. 1):");
  while(Serial.available()==0);
  chip_number = Serial.parseInt();
  serialFlush();
  Serial.print(F("Chip Number: ")); Serial.println(chip_number);

  Serial.println();
  Serial.println(F("Turn on 24 V supply to solenoid valves."));
  set_valve(source,1);
  Serial.println(F("Source is turned ON."));
  Serial.print(F("After pressing 'enter', increase Source pressure to ")); Serial.print(hi_pressure);
  Serial.println(F(" psi using precision regulator."));
  while(Serial.read()!=10); //wait for user to press enter.
  while(source_pressure < 15){
    source_pressure = analogRead(source_pressure_input)*lin_slope[source_pressure_input]+lin_yint[source_pressure_input];
    manifold_pressure = analogRead(manifold_pressure_input)*lin_slope[manifold_pressure_input]+lin_yint[manifold_pressure_input];
    Serial.print("Manifold Pressure: "); Serial.print(manifold_pressure); Serial.print(" psi --- ");
    Serial.print("Source Pressure: "); Serial.print(source_pressure); Serial.println(" psi  <-- Increase Source Pressure to 16 psi");
  }
  Serial.println();
  Serial.println(F("Boot up AmScope & click MU043M under 'Camera List'. "));
  Serial.println(F("Under 'Exposure & Gain', turn OFF 'Auto Explosure' & set:"));
  Serial.println(F("    Exposure Time = 0.5 ms"));
  Serial.println(F("    Gain = 100%"));
  Serial.println(F("Move XY-Positioning stage to desired fluidic element and press enter."));
  while(Serial.read()!=10); //wait for user to press enter.
  serialFlush();
  
//  delay(1000);
  
}

// --------------- set_valve --------------- //
// Takes as input valve pair indicated manifold and valve in that manifold
void set_valve(byte valve_pair[], boolean val){
  bitWrite(valve[valve_pair[0]],7-valve_pair[1],val);
  update_valves();
}

// --------------- update_valves --------------- //
// Pushes byte array "valve" to shift registers to set the state of the electronic valves
void  update_valves() {
  digitalWrite(RCLK_Pin, LOW);

  for (int i = num_manifolds - 1; i >= 0; i--) {
    shiftOut(SER_Pin,SRCLK_Pin,LSBFIRST,valve[i]);
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
void serialFlush(){
  while(Serial.available()>0) Serial.read();
}
