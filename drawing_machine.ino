// libraries
#include <LiquidCrystal.h> // LCD
#include <SPI.h>           // SD card
#include <SD.h>            // SD card

// LCD variables - https://www.arduino.cc/en/Reference/LiquidCrystalConstructor
const int rs = 12, en = 11, d4 = 9, d5 = 8, d6 = 7, d7 = 6; // 2, 3, 4, 5 become 6, 7, 8, 9
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// SD vars
// const int SD_CS = 18, SD_CLK = 19 , SD_MOSI = 20, SD_MISO = 21;
// File dataFile;
// const String DATA_FILE = "data.txt";
// String saved_data;
// int saved_data; // = "0,71,20,996.40,15";

// motors
const int ARM2    = 2;
const int PLATTER = 3;
const int ARM1    = 4;
bool motorsOn = false;

// waves
const char *WAVES[] = {"No", "Sq", "Sa",  "Tr", "Si"};
const int WAVE_NONE     = 0;
const int WAVE_SQUARE   = 1;
const int WAVE_SAW      = 2;
const int WAVE_TRIANGLE = 3;
const int WAVE_SINE     = 4;

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

unsigned int button_a_amp_up = 0, button_a_amp_down = 0, button_a_period_up = 0, button_a_period_down = 0,
             button_b_amp_up = 0, button_b_amp_down = 0, button_b_period_up = 0, button_b_period_down = 0;
unsigned int selector_a_1 = 0, selector_a_2 = 0, selector_a_3 = 0, selector_a_4 = 0, selector_a_5 = 0,
             selector_b_1 = 0, selector_b_2 = 0, selector_b_3 = 0, selector_b_4 = 0, selector_b_5 = 0;
long selectorMillis = 0;

const int  BUTTON_PAUSE = 50; // milliseconds to wait while button being pressed

// variables
int PlatterSpeed = 60; // initial variables, will change
int Arm1Speed    = 150;
int Arm2Speed    = 150;
int new_Arm1Speed, new_Arm2Speed;

int factors[3];

unsigned int adjusted_platter_speed;
unsigned int arm1_adjustment;
unsigned int prev_arm1_adjustment;
unsigned int arm2_adjustment;
unsigned int prev_arm2_adjustment;
unsigned int duration, duration_s;
unsigned int arm1_remainder;
unsigned int arm2_remainder;
float period_slope;
int adjustment;

// motor speed
const int MAX_SPEED = 255;
const int MIN_SPEED = 55;

const int MIN_AMP    = 5;
const int MAX_AMP    = 100;
const int MIN_PERIOD = 5;
const int MAX_PERIOD = 300;

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

String get_rotation(double voltage, int motor) {
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
  int precision = 2 - int(log10(rotation));
  return String(rotation, precision);
}

String get_status() {
  String arm1_rot = get_rotation(Arm1Speed, ARM1);
  String arm2_rot = get_rotation(Arm2Speed, ARM2);
  String platter_rot = get_rotation(PlatterSpeed, PLATTER);

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
  delay(100);

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

int square_adjustment(int remainder, int period, int amp) { // remainder in millis, period in secs, amp is int
  /*  |_____     |   firast half HIGH, second half LOW
      |    |     |
      |    |_____|
      | _____ ___|*/
  int period_millis = period * 1000;
  if (remainder <= period_millis / 2) { // HIGH
    adjustment = int(amp / 2);
  } else { // LOW
    adjustment = -int(amp / 2);
  }
  //Serial.println("rem: " + String(remainder) + ", period: " + String(period) + ", amp: " + String(amp) +",  adj: " + String(adjustment));
  return adjustment;
}

int saw_adjustment(int remainder, int period, int amp) { // remainder in millis, period in secs, amp is int
  /*   |   /|   ramps up, then snaps back
       |__/_|
       | /  |
       |/___| */
  int period_millis = period * 1000;
  period_slope = amp / period_millis;
  adjustment = int((-amp / 2) + period_slope * remainder);
  Serial.println("rem: " + String(remainder) + ", period: " + String(period) + ", amp: " + String(amp) + ",  adj: " + String(adjustment));
  return adjustment;
}

int triangle_adjustment(int remainder, int period, int amp) { // remainder in millis, period in secs, amp is int
  /*   |   /\        ramps up for half the period, then ramps back down
       |__/__\___
       | /    \
       |/______\__ */
  int period_millis = period * 1000;
  period_slope = 2 * (amp / period_millis); // half the period so twice the period_slope
  if (remainder <= period_millis / 2) { // ASCENDING
    Serial.println("up");
    adjustment = int((-amp / 2) + (period_slope * remainder));
  } else { // DESCENDING
    Serial.println("down");
    adjustment = int((amp / 2) - (period_slope * (remainder / 2)));
  }
  // Serial.println("rem: " + String(remainder) + ", period: " + String(period) + ", amp: " + String(amp) + ",  adj: " + String(adjustment));
  return adjustment;
}

int sine_adjustment(int remainder, int period, int amp) { // remainder in millis, period in secs, amp is int
  int period_millis = period * 1000;
  float angular_progress = 360 * (remainder / period_millis);
  adjustment = int((-amp / 2) + sin(angular_progress) * amp);
  Serial.println("rem: " + String(remainder) + ", period: " + String(period) + ", amp: " + String(amp) + ",  adj: " + String(adjustment));
  return adjustment;
}


void loop() {

  start_button = digitalRead(START_BUTTON);
  if (start_button == ON && motorsOn == true) { // the motors are running and might need adjustment

    currentMillis = millis();
    duration = (currentMillis - startMillis);
    arm1_remainder = duration % (arm1_period * 1000);
    arm2_remainder = duration % (arm2_period * 1000);
    //Serial.println("arm1 period: " + String(arm1_period) + ", arm2 period:  " + String(arm2_period));
    Serial.println("arm1 period: " + String(arm1_period) + " duration: " + String(duration) + " arm1 remainder: " + String(arm1_remainder)  );

    if      (arm1_wave == WAVE_NONE)      arm1_adjustment = 0;
    else if (arm1_wave == WAVE_SQUARE)    arm1_adjustment = square_adjustment(    arm1_remainder, arm1_period, arm1_amp);
    else if (arm1_wave == WAVE_SAW)       arm1_adjustment = saw_adjustment(       arm1_remainder, arm1_period, arm1_amp);
    else if (arm1_wave == WAVE_TRIANGLE)  arm1_adjustment = triangle_adjustment(  arm1_remainder, arm1_period, arm1_amp);
    else if (arm1_wave == WAVE_SINE)      arm1_adjustment = sine_adjustment (     arm1_remainder, arm1_period, arm1_amp);

    if (arm1_adjustment != prev_arm1_adjustment) { // only change sopeeds when we need to
      new_Arm1Speed = Arm1Speed + arm1_adjustment;
      if (new_Arm1Speed > MAX_SPEED)      analogWrite(ARM1, MAX_SPEED);
      else if (new_Arm1Speed < MIN_SPEED) analogWrite(ARM1, 0);
      else                                analogWrite(ARM1, new_Arm1Speed);
      prev_arm1_adjustment = arm1_adjustment;
    }

    Serial.println("Arm1Speed: " + String(Arm1Speed) + " Adj arm speed: " + String(arm1_adjustment));

    if      (arm2_wave == WAVE_NONE)      arm2_adjustment = 0;
    else if (arm2_wave == WAVE_SQUARE)    arm2_adjustment = square_adjustment(    arm2_remainder, arm2_period, arm2_amp);
    else if (arm2_wave == WAVE_SAW)       arm2_adjustment = saw_adjustment(       arm2_remainder, arm2_period, arm2_amp);
    else if (arm2_wave == WAVE_TRIANGLE)  arm2_adjustment = triangle_adjustment(  arm2_remainder, arm2_period, arm2_amp);
    else if (arm2_wave == WAVE_SINE)      arm2_adjustment = sine_adjustment (     arm2_remainder, arm2_period, arm2_amp);

    if (arm2_adjustment != prev_arm2_adjustment) { // only change sopeeds when we need to
      new_Arm2Speed = Arm2Speed + arm2_adjustment;
      if (new_Arm2Speed > MAX_SPEED)      analogWrite(ARM2, MAX_SPEED);
      else if (new_Arm2Speed < MIN_SPEED) analogWrite(ARM2, 0);
      else                                analogWrite(ARM2, new_Arm2Speed);
      prev_arm2_adjustment = arm2_adjustment;
    }

    Serial.println("Arm2Speed: " + String(Arm2Speed) + " Adj arm speed: " + String(arm2_adjustment));
  }

  /*
     Start the motors
  */

  if (start_button == ON && motorsOn == false) { // start the motors
    /*
        Necessary? The starting voltages might be incorrect depending on waves function choices
        and the wave function now writes to the motor controls

      analogWrite(PLATTER, PlatterSpeed);
      analogWrite(ARM1, Arm1Speed);
      analogWrite(ARM2, Arm2Speed);
      Serial.println("motors started");
    */

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

    if (button_1_state == ON && Arm1Speed < MAX_SPEED)         Arm1Speed += 1;
    else if (button_2_state == ON && Arm1Speed > MIN_SPEED)         Arm1Speed -= 1;

    if (button_4_state == ON && PlatterSpeed < MAX_SPEED)   PlatterSpeed += 1;
    else if (button_5_state == ON && PlatterSpeed > MIN_SPEED)      PlatterSpeed -= 1;

    if (button_7_state == ON && Arm2Speed < MAX_SPEED)         Arm2Speed += 1;
    else if (button_8_state == ON && Arm2Speed > MIN_SPEED)         Arm2Speed -= 1;

    // advance Arms
    if (button_3_state == ON) { // Arm1
      int advance_speed = int(Arm1Speed / 2);
      if (advance_speed < MIN_SPEED) advance_speed = MIN_SPEED;
      analogWrite(ARM1, advance_speed);
      while (digitalRead(BUTTON_3) == ON);
      analogWrite(ARM1, 0);
    }

    if (button_9_state == ON) { // Arm2
      int advance_speed = int(Arm2Speed / 2);
      if (advance_speed < MIN_SPEED) advance_speed = MIN_SPEED;
      analogWrite(ARM2, advance_speed);
      while (digitalRead(BUTTON_9) == ON);
      analogWrite(ARM2, 0);
    }

    // set the waves
    selector_a_1 = digitalRead(SELECTOR_A_1); // WAVE_NONE
    selector_a_2 = digitalRead(SELECTOR_A_2); // WAVE_SQUARE
    selector_a_3 = digitalRead(SELECTOR_A_3); // WAVE_SAW
    selector_a_4 = digitalRead(SELECTOR_A_4); // WAVE_TRIANGLE
    selector_a_5 = digitalRead(SELECTOR_A_5); // WAVE_SINE
    selector_b_1 = digitalRead(SELECTOR_B_1); // WAVE_NONE
    selector_b_2 = digitalRead(SELECTOR_B_2); // WAVE_SQUARE
    selector_b_3 = digitalRead(SELECTOR_B_3); // WAVE_SAW
    selector_b_4 = digitalRead(SELECTOR_B_4); // WAVE_TRIANGLE
    selector_b_5 = digitalRead(SELECTOR_B_5); // WAVE_SINE

    if (selector_a_1 == ON)         arm1_wave = WAVE_NONE;
    else if (selector_a_2 == ON)    arm1_wave = WAVE_SQUARE;
    else if (selector_a_3 == ON)    arm1_wave = WAVE_SAW;
    else if (selector_a_4 == ON)    arm1_wave = WAVE_TRIANGLE;
    else if (selector_a_5 == ON)    arm1_wave = WAVE_SINE;

    if (selector_b_1 == ON)         arm2_wave = WAVE_NONE;
    else if (selector_b_2 == ON)    arm2_wave = WAVE_SQUARE;
    else if (selector_b_3 == ON)    arm2_wave = WAVE_SAW;
    else if (selector_b_4 == ON)    arm2_wave = WAVE_TRIANGLE;
    else if (selector_b_5 == ON)    arm2_wave = WAVE_SINE;


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
