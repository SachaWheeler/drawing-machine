// libraries
#include <LiquidCrystal.h> // LCD
#include <SPI.h>           // SD card
#include <SD.h>            // SD card

// LCD variables - https://www.arduino.cc/en/Reference/LiquidCrystalConstructor
const int rs = 12, en = 11, d4 = 9, d5 = 8, d6 = 7, d7 = 6; // 2, 3, 4, 5 become 6, 7, 8, 9
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// SD vars
const int SD_CS = 18, SD_CLK = 19 , SD_MOSI = 20, SD_MISO = 21;
File dataFile;
const String DATA_FILE = "data.txt";
//String saved_data;
int saved_data; // = "0,71,20,996.40,15";

// motors
const int ARM2    = 2;
const int PLATTER = 3;
const int ARM1    = 4;
bool motorsOn = false;

// waves
const char *WAVES[] = {"No", "Sq", "Sa",  "Tr", "Si"};
const int WAVE_NONE     = 1;
const int WAVE_SQUARE   = 2;
const int WAVE_SAW      = 3;
const int WAVE_TRIANGLE = 4;
const int WAVE_SINE     = 5;

const byte NONE[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

const byte SQUARE[8] = {
  0b00000,
  0b11111,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b00000,
  0b00000
};

const byte SAW[8] = {
  0b00000,
  0b10000,
  0b11000,
  0b10100,
  0b10010,
  0b10001,
  0b00000,
  0b00000
};

const byte TRIANGLE[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b01010,
  0b10001,
  0b00000,
  0b00000,
  0b00000
};

const byte SINE[8] = {
  0b00000,
  0b00000,
  0b00011,
  0b00100,
  0b01000,
  0b11000,
  0b00000,
  0b00000
};

// control panel 1
const int  START_BUTTON = 34;
const int  BUTTON_1 = 36;     // row 1 col 1
const int  BUTTON_2 = 38;     // row 2 col 1
const int  BUTTON_3 = 40;     // row 3 col 1
const int  BUTTON_4 = 42;     // row 1 col 2
const int  BUTTON_5 = 44;     // row 2 col 2
const int  BUTTON_6 = 46;     // row 3 col 2
const int  BUTTON_7 = 48;     // row 1 col 3
const int  BUTTON_8 = 50;     // row 2 col 3
const int  BUTTON_9 = 52;     // row 3 col 3

// control panel 2
const int  SELECTOR_A_1 = 53; // first selector, first option
const int  SELECTOR_A_2 = 51;
const int  SELECTOR_A_3 = 49;
const int  SELECTOR_A_4 = 47;
const int  SELECTOR_A_5 = 45;
const int  SELECTOR_B_1 = 43;
const int  SELECTOR_B_2 = 41;
const int  SELECTOR_B_3 = 39;
const int  SELECTOR_B_4 = 37;
const int  SELECTOR_B_5 = 35;

const int  BUTTON_A_AMP_UP = 22;
const int  BUTTON_A_AMP_DOWN = 24;
const int  BUTTON_A_PERIOD_UP = 26;
const int  BUTTON_A_PERIOD_DOWN = 28;
const int  BUTTON_B_AMP_UP = 23;
const int  BUTTON_B_AMP_DOWN = 25;
const int  BUTTON_B_PERIOD_UP = 27;
const int  BUTTON_B_PERIOD_DOWN = 29;

int button_a_amp_up = 0, button_a_amp_down = 0, button_a_period_up = 0, button_a_period_down = 0,
    button_b_amp_up = 0, button_b_amp_down = 0, button_b_period_up = 0, button_b_period_down = 0;
int selector_a_1 = 0, selector_a_2 = 0, selector_a_3 = 0, selector_a_4 = 0, selector_a_5 = 0,
    selector_b_1 = 0, selector_b_2 = 0, selector_b_3 = 0, selector_b_4 = 0, selector_b_5 = 0;
long selectorMillis = 0;

const int  BUTTON_PAUSE = 50; // milliseconds to wait while button being pressed

// variables
int PlatterSpeed = 60; // initial variables, will change
int Arm1Speed    = 150;
int Arm2Speed    = 150;

int factors[3];
int precision;

unsigned int adjusted_platter_speed;
unsigned int adjusted_arm1_speed;
unsigned int adjusted_arm2_speed;
unsigned int duration;
unsigned int remainder;
float period_slope;
unsigned int adjustment;

const int  MAX_PLATTER_SPEED = 255;
const int  MAX_ARM1_SPEED    = 255;
const int  MAX_ARM2_SPEED    = 255;
const int  MOTOR_MIN         = 55;

const int  MIN_AMP    = 5;
const int MAX_AMP    = 100;
const int MIN_PERIOD = 1;
const int MAX_PERIOD = 120;

int start_button;

int button_1_state = 0, button_2_state = 0, button_3_state = 0,
    button_4_state = 0, button_5_state = 0, button_6_state = 0,
    button_7_state = 0, button_8_state = 0, button_9_state = 0;

unsigned int arm1_wave = 0, arm1_period = MIN_PERIOD, arm1_amp = MIN_AMP,
             arm2_wave = 0, arm2_period = MIN_PERIOD, arm2_amp = MIN_AMP;
unsigned int prev_arm1_wave = arm1_wave,
             prev_arm2_wave = arm2_wave;

unsigned long startMillis, stopMillis, currentMillis;
const unsigned long PERIOD = 100;  // milliseconds

const bool ON = LOW,
           OFF = HIGH;

String get_rot_from_voltage(double voltage, int motor) {
  // RPM = 1.01 * x + 0.857, R^2 = 0.986 - https://docs.google.com/spreadsheets/d/1f9QExcumMRH8idaQd5ZxFR9PZowoW2o2FPOhNIjPI1c/edit#gid=1207148320
  double x = log10(voltage);
  if (motor == ARM1) {
    // 235 + -203x + 45x^2 where x is log10(voltage)
    factors[0] = 235;
    factors[1] = -203;
    factors[2] = 45;
  } else if (motor == ARM2) {
    // 235 + -203x + 45x^2 where x is log10(voltage)
    factors[0] = 235;
    factors[1] = -203;
    factors[2] = 45;
  } else if (motor == PLATTER) {
    // 2173 + -1766x + 384x^2 where x is log10(voltage)
    factors[0] = 2173;
    factors[1] = -1766;
    factors[2] = 384;
  }

  double rotation = factors[0] + (factors[1] * x) + (factors[2] * sq(x));

  if (motorsOn) precision = 0;
  else          precision = 2 - int(log10(rotation));

  //Serial.println(String(voltage) + " " + String(rotation) + " " + String(precision) + " " + String(rotation, precision));
  return String(rotation, precision);
}

String get_status() {
  String arm1_rot = get_rot_from_voltage(Arm1Speed, ARM1);
  String arm2_rot = get_rot_from_voltage(Arm2Speed, ARM2);
  String platter_rot = get_rot_from_voltage(PlatterSpeed, PLATTER);

  String str = String(arm1_rot) + " " + String(platter_rot) + " " + String(arm2_rot);
  if (motorsOn) {
    str += " " + duration;
  }
  return str;
}

String get_wave_status() {
  return "" + String(WAVES[arm1_wave]) + "," + String(arm1_amp) + "," + String(arm1_period) + " " + String(WAVES[arm2_wave]) + "," + String(arm2_amp) + "," + String(arm2_period);
}

unsigned int get_amp(int amplitude) {
  if (amplitude > MAX_AMP) return MAX_AMP;
  else if (amplitude < MIN_AMP) return MIN_AMP;
  else return amplitude;
}

unsigned int get_period(int period) {
  if (period > MAX_PERIOD) return MAX_PERIOD;
  else if (period < MIN_PERIOD) return MIN_PERIOD;
  else return period;
}

void lcd_display(String line_1, String line_2) {
  lcd.setCursor(0, 0);
  lcd.print(line_1);
  lcd.setCursor(0, 1);
  lcd.print(line_2);
}

/*
  String get_data_format() {
  return String(PlatterSpeed) + "," + String(Arm1Speed) + "," + String(arm1_period) + "," + String(arm1_amp) + "," + String(Arm2Speed) + "," + String(arm2_period) + "," + String(arm2_amp);
  }*/

void setup() {
  // LCD
  lcd.begin(16, 2);
  //lcd.createChar(0, SQUARE);

  // SD card
  // SD.begin(SD_CS);

  // buttons
  pinMode(START_BUTTON,         INPUT_PULLUP);

  // pause startup if START_BUTTON is pushed
  if (digitalRead(START_BUTTON) == ON) {
    lcd_display("Start button", "must be OFF!");
    while (digitalRead(START_BUTTON) == ON); // wait until turned off
    lcd.clear();
  }
  lcd_display("Starting...", " ");

  pinMode(BUTTON_1,             INPUT_PULLUP);
  pinMode(BUTTON_2,             INPUT_PULLUP);
  pinMode(BUTTON_3,             INPUT_PULLUP);
  pinMode(BUTTON_4,             INPUT_PULLUP);
  pinMode(BUTTON_5,             INPUT_PULLUP);
  pinMode(BUTTON_6,             INPUT_PULLUP);
  pinMode(BUTTON_7,             INPUT_PULLUP);
  pinMode(BUTTON_8,             INPUT_PULLUP);
  pinMode(BUTTON_9,             INPUT_PULLUP);

  pinMode(SELECTOR_A_1,         INPUT_PULLUP);
  pinMode(SELECTOR_A_2,         INPUT_PULLUP);
  pinMode(SELECTOR_A_3,         INPUT_PULLUP);
  pinMode(SELECTOR_A_4,         INPUT_PULLUP);
  pinMode(SELECTOR_A_5,         INPUT_PULLUP);
  pinMode(SELECTOR_B_1,         INPUT_PULLUP);
  pinMode(SELECTOR_B_2,         INPUT_PULLUP);
  pinMode(SELECTOR_B_3,         INPUT_PULLUP);
  pinMode(SELECTOR_B_4,         INPUT_PULLUP);
  pinMode(SELECTOR_B_5,         INPUT_PULLUP);

  pinMode(BUTTON_A_AMP_UP,      INPUT_PULLUP);
  pinMode(BUTTON_A_AMP_DOWN,    INPUT_PULLUP);
  pinMode(BUTTON_A_PERIOD_UP,   INPUT_PULLUP);
  pinMode(BUTTON_A_PERIOD_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_B_AMP_UP,      INPUT_PULLUP);
  pinMode(BUTTON_B_AMP_DOWN,    INPUT_PULLUP);
  pinMode(BUTTON_B_PERIOD_UP,   INPUT_PULLUP);
  pinMode(BUTTON_B_PERIOD_DOWN, INPUT_PULLUP);

  // motors
  pinMode(ARM1, OUTPUT);
  pinMode(ARM2, OUTPUT);
  pinMode(PLATTER, OUTPUT);

  Serial.begin(9600);
  while (!Serial); // wait until Serial is available

  /*

    // is there data to read? // PlatterSpeed, Arm1Speed, arm1_period, arm1_amp, Arm2Speed, arm2_period, arm2_amp,
    //if (SD.exists(DATA_FILE)) {
    //    Serial.write("we have a data file");
    dataFile = SD.open(DATA_FILE);
    Serial.write(dtaFile);
    if (dataFile) {
    while (dataFile.available()) {
      saved_data = dataFile.read();
    }
    Serial.write(saved_data);
    dataFile.close();
    }
    //  }else{
    //Serial.write("no data file");
    //dataFile = SD.open(DATA_FILE, FILE_WRITE);
    //Serial.println(get_data_format());
    //dataFile.println(get_data_format());
    //dataFile.close();
    //Serial.println("done writing.");
    //}

    write using: dataFile.write(data);

      dataFile.print(data);
      dataFile.println(data); // followed by a new line

      dataFile.read();

      if datafile exists:
        read data from file
        parse data
        set speeds, period and aplitudes from data

        int PlatterSpeed = 60; // initial variables, will change
        int Arm1Speed    = 150;
        int Arm2Speed    = 150;

        unsigned int arm1_period = MIN_PERIOD, arm1_amp = MIN_AMP,
              arm2_period = MIN_PERIOD, arm2_amp = MIN_AMP;
  */

  lcd_display(get_status(), get_wave_status());
}

void loop() {

  start_button = digitalRead(START_BUTTON);
  if (start_button == ON && motorsOn == true) { // the motors are running and might need adjustment
    /*
       This is where we might adjust the speeds of
       Arm1Speed and Arm2Speed according to:
          waveforms (none, square, saw, triangle, sine)
          frequency
          amplitude
       based on startMillis

       none:
           return current speed
       square:
           find out how many freqs there have been since startMillis
           odd, return MAX_AMP, even, return MIN_AMP
       saw:
           start at MAX_AMP, ramp down until MIN_AMP
           snap back to MAX_AMP
       triangle:
           ramp up, then ramp down
           find out how many freqs there have been since startMillis
           in the first half of frequency? ramp up until MAX_AMP
           then ramp down until MIN_AMP
       sine:
           plt the sine of currentMillis - startMillis
    */

    currentMillis = millis();
    duration = int(currentMillis - startMillis);
    remainder = duration % (arm1_period);

    //Serial.println(wave_status);
    Serial.println("remainder: " + String(remainder));

    if (arm1_wave == WAVE_NONE) { // no wave
      adjusted_arm1_speed = Arm1Speed;
    } else if (arm1_wave == WAVE_SQUARE) { // square - DONE

      if (remainder <= int(arm1_period / 2)) { // HIGH
        adjustment = int(arm1_amp / 2);
      } else { // LOW
        adjustment = -int(arm1_amp / 2);
      }
      adjusted_arm1_speed = Arm1Speed + adjustment;

    } else if (arm1_wave == WAVE_SAW) { // saw DONE
      // ramps up, then snaps back
      period_slope = 1000 * arm1_amp / arm1_period;
      //Serial.println(period_slope);
      adjustment = -int(arm1_amp / 2) + int(period_slope * remainder / 1000);
      adjusted_arm1_speed = Arm1Speed + adjustment;

    } else if (arm1_wave == WAVE_TRIANGLE) { // triangle
      /*
              |   /\
              |__/__\___
              | /    \
              |/______\_________
      */
      // ramps up for half the period, then ramps back down
      period_slope = 2 * 1000 * arm1_amp / arm1_period; // half the period so twice the period_slope
      if (remainder <= int(arm1_period / 2)) { // ASCENDING
        Serial.println("up");
        adjustment = -int(arm1_amp / 2) + int(period_slope * remainder / 1000);
        Serial.println(remainder);
      } else { // DESCENDING
        Serial.println("down");
        adjustment = -int(arm1_amp / 2) + int(arm1_amp - period_slope * arm1_period / 1000);
        Serial.println(arm1_period - remainder);
      }
      adjusted_arm1_speed = Arm1Speed + adjustment;



    } else if (arm1_wave == WAVE_SINE) { // sine

      adjusted_arm1_speed = Arm1Speed;

    }
    Serial.println("ArmSpeed: " + String(Arm1Speed) + " Adj arm speed: " + String(adjusted_arm1_speed));
  }

  /*
     Start the motors
  */

  if (start_button == ON && motorsOn == false) { // start the motors
    analogWrite(PLATTER, PlatterSpeed);
    analogWrite(ARM1, Arm1Speed);
    analogWrite(ARM2, Arm2Speed);
    Serial.println("motors started");

    motorsOn = true;
    lcd_display(get_status(), get_wave_status());
    startMillis = millis();  // start time
  }

  /*
     Stop the motors
  */

  if (start_button == OFF) {
    if (motorsOn == true) { // stop the motors
      analogWrite(PLATTER, 0);
      analogWrite(ARM1, 0);
      analogWrite(ARM2, 0);
      Serial.println("motors stopped");

      motorsOn = false;
      stopMillis = millis();  // stop time
    }

    // set the speeds
    button_1_state = digitalRead(BUTTON_1);
    button_2_state = digitalRead(BUTTON_2);
    button_3_state = digitalRead(BUTTON_3);
    button_4_state = digitalRead(BUTTON_4);
    button_5_state = digitalRead(BUTTON_6); // changed from BUTTON_5 due to wiring issue
    button_6_state = digitalRead(BUTTON_6); // currently serving BUTTON_5 data
    button_7_state = digitalRead(BUTTON_7);
    button_8_state = digitalRead(BUTTON_8);
    button_9_state = digitalRead(BUTTON_9);

    if (button_1_state == ON && Arm1Speed < MAX_ARM1_SPEED)         Arm1Speed += 1;
    else if (button_2_state == ON && Arm1Speed > MOTOR_MIN)         Arm1Speed -= 1;

    if (button_4_state == ON && PlatterSpeed < MAX_PLATTER_SPEED)   PlatterSpeed += 1;
    else if (button_5_state == ON && PlatterSpeed > MOTOR_MIN)      PlatterSpeed -= 1;

    if (button_7_state == ON && Arm2Speed < MAX_ARM2_SPEED)         Arm2Speed += 1;
    else if (button_8_state == ON && Arm2Speed > MOTOR_MIN)         Arm2Speed -= 1;

    // advance Arms
    if (button_3_state == ON) { // Arm1
      int advance_speed = int(Arm1Speed / 2);
      if (advance_speed < MOTOR_MIN) advance_speed = MOTOR_MIN;
      analogWrite(ARM1, advance_speed);
      while (digitalRead(BUTTON_3) == ON);
      analogWrite(ARM1, 0);
    }

    if (button_9_state == ON) { // Arm2
      int advance_speed = int(Arm2Speed / 2);
      if (advance_speed < MOTOR_MIN) advance_speed = MOTOR_MIN;
      analogWrite(ARM2, advance_speed);
      while (digitalRead(BUTTON_9) == ON);
      analogWrite(ARM2, 0);
    }

    // set the waves
    selector_a_1 = digitalRead(SELECTOR_A_1);
    selector_a_2 = digitalRead(SELECTOR_A_2);
    selector_a_3 = digitalRead(SELECTOR_A_3);
    selector_a_4 = digitalRead(SELECTOR_A_4);
    selector_a_5 = digitalRead(SELECTOR_A_5);
    selector_b_1 = digitalRead(SELECTOR_B_1);
    selector_b_2 = digitalRead(SELECTOR_B_2);
    selector_b_3 = digitalRead(SELECTOR_B_3);
    selector_b_4 = digitalRead(SELECTOR_B_4);
    selector_b_5 = digitalRead(SELECTOR_B_5);

    if (selector_a_1 == ON)         arm1_wave = 0;
    else if (selector_a_2 == ON)    arm1_wave = 1;
    else if (selector_a_3 == ON)    arm1_wave = 2;
    else if (selector_a_4 == ON)    arm1_wave = 3;
    else if (selector_a_5 == ON)    arm1_wave = 4;

    if (selector_b_1 == ON)         arm2_wave = 0;
    else if (selector_b_2 == ON)    arm2_wave = 1;
    else if (selector_b_3 == ON)    arm2_wave = 2;
    else if (selector_b_4 == ON)    arm2_wave = 3;
    else if (selector_b_5 == ON)    arm2_wave = 4;


    button_a_amp_up      = digitalRead(BUTTON_A_AMP_UP);
    button_a_amp_down    = digitalRead(BUTTON_A_AMP_DOWN);
    button_a_period_up   = digitalRead(BUTTON_A_PERIOD_UP);
    button_a_period_down = digitalRead(BUTTON_A_PERIOD_DOWN);
    button_b_amp_up      = digitalRead(BUTTON_B_AMP_UP);
    button_b_amp_down    = digitalRead(BUTTON_B_AMP_DOWN);
    button_b_period_up   = digitalRead(BUTTON_B_PERIOD_UP);
    button_b_period_down = digitalRead(BUTTON_B_PERIOD_DOWN);

    if (button_a_amp_up == ON && arm1_amp < MAX_AMP)                    arm1_amp += 1;
    else if (button_a_amp_down == ON && arm1_amp > MIN_AMP)             arm1_amp -= 1;

    if (button_a_period_up == ON && arm1_period < MAX_PERIOD)           arm1_period += 1;
    else if (button_a_period_down == ON && arm1_period > MIN_PERIOD)    arm1_period -= 1;

    if (button_b_amp_up == ON && arm2_amp < MAX_AMP)                    arm2_amp += 1;
    else if (button_b_amp_down == ON && arm2_amp > MIN_AMP)             arm2_amp -= 1;

    if (button_b_period_up == ON && arm2_period < MAX_PERIOD)           arm2_period += 1;
    else if (button_b_period_down == ON && arm2_period > MIN_PERIOD)    arm2_period -= 1;

    if (button_a_amp_up + button_a_amp_down +
        button_a_period_up + button_a_period_down +
        button_b_amp_up + button_b_amp_down +
        button_b_period_up + button_b_period_down +
        button_1_state + button_2_state + button_4_state + button_5_state + button_7_state + button_8_state < 14 ||
        arm1_wave != prev_arm1_wave || arm2_wave != prev_arm2_wave) {
      // a button has been pressed
      prev_arm1_wave = arm1_wave;
      prev_arm2_wave = arm2_wave;

      lcd_display(get_status(), get_wave_status());
      delay(100);
    }
  }
}
