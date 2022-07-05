// libraries
#include <LiquidCrystal.h> // LCD
#include <SPI.h>           // SD card
#include <SD.h>            // SD card

#define TWO_PI 6.283185307179586476925286766559

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
int PlatterSpeed = 255; // initial variables, will change
int Arm1Speed    = 255;
int Arm2Speed    = 150;
int new_Arm1Speed, new_Arm2Speed;

int factors[3];

unsigned int adjusted_platter_speed;
int arm1_adjustment;
int prev_arm1_adjustment;
int arm2_adjustment;
int prev_arm2_adjustment;
unsigned int duration, duration_s;
unsigned int arm1_remainder;
unsigned int arm2_remainder;
float period_slope;
int adjustment;

// motor speed
const int MAX_SPEED = 255;
const int MIN_SPEED = 55;

const int MIN_AMP        = 50;
const int MAX_AMP        = 200;
const int MIN_PERIOD     = 5;
const int MAX_PERIOD     = 300;
const int DEFAULT_AMP    = 50;
const int DEFAULT_PERIOD = 20;

int start_button;

unsigned int button_1_state = 0, button_2_state = 0, button_3_state = 0,
             button_4_state = 0, button_5_state = 0, button_6_state = 0,
             button_7_state = 0, button_8_state = 0, button_9_state = 0;

unsigned int arm1_wave = 0, arm1_period = DEFAULT_PERIOD, arm1_amp = DEFAULT_AMP, // these defaults will not be required when/if SD card saving is enabled
             arm2_wave = 0, arm2_period = DEFAULT_PERIOD, arm2_amp = DEFAULT_AMP;
unsigned int prev_arm1_wave = arm1_wave,
             prev_arm2_wave = arm2_wave;

unsigned long startMillis, stopMillis, currentMillis, prevMillis;
const unsigned long PERIOD = 100;  // milliseconds

const bool ON = LOW,
           OFF = HIGH;

String get_status() {
  String arm1_rot = String(int(Arm1Speed));
  // String arm2_rot = get_rotation(Arm2Speed, ARM2);
  String platter_rot = String(int(PlatterSpeed));

  String str = String(arm1_rot) + " " + String(platter_rot) + " " + String(int((currentMillis - startMillis) / 1000)) + "s";
  for (int i = 0; i < (16 - str.length()); i++) {
    str += ' ';
  }
  return str;
}

String get_wave_status() {
  String str = String(WAVES[arm1_wave]) + "," + String(arm1_amp) + "," + String(arm1_period); + " " + String(duration);
  //String(WAVES[arm2_wave]) + "," + String(arm2_amp) + "," + String(arm2_period);
  for (int i = 0; i < (16 - str.length()); i++) {
    str += ' ';
  }
  return str;
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

void setup() {
  // LCD
  lcd.begin(16, 2);

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

  Serial.begin(230400);
  while (!Serial); // wait until Serial is available

  prevMillis = 0;

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
  return adjustment;
}

int saw_adjustment(int remainder, int period, int amp) { // remainder in millis, period in secs, amp is int
  /*   |   /|   ramps up, then snaps back
       |__/_|
       | /  |
       |/___| */
  period_slope = amp / period;
  adjustment = int((-amp / 2) + period_slope * remainder / 1000);
  //Serial.println("slope: " + String(period_slope) + ", adj: " + String(adjustment) + ", period: " + String(period) + ", amp: " + String(amp));
  return adjustment;
}

int triangle_adjustment(int remainder, int period, int amp) { // remainder in millis, period in secs, amp is int
  /*   |   /\        ramps up for half the period, then ramps back down
       |__/__\___
       | /    \
       |/______\__ */
  period_slope = 2 * (amp / period); // half the period so twice the period_slope
  if (remainder <= period * 1000 / 2) { // ASCENDING
    adjustment = int((-amp / 2) + period_slope * remainder / 1000);
  } else { // DESCENDING
    adjustment = int((3 * amp / 2) - period_slope * remainder / 1000);
  }
  //Serial.println("slope: " + String(period_slope) + ", adj: " + String(adjustment) + ", period: " + String(period) + ", amp: " + String(amp));
  return adjustment;
}

int sine_adjustment(int remainder, int period, int amp) { // remainder in millis, period in secs, amp is int
  // 1 radian = 180°/ π = 57.30°.
  int period_millis = period * 1000;
  float angular_progress = TWO_PI * (remainder / period_millis);
  Serial.println("millis: " + String(period_millis) + ",  (remainder / period_millis): " + String(360 * remainder / period_millis) );
  adjustment = int(amp * sin(angular_progress));
  Serial.println("rem: " + String(remainder) + ", period: " + String(period) + ", amp: " + String(amp) + ", angle: " + String(angular_progress) + ",  adj: " + String(adjustment));
  return adjustment;
}


void loop() {

  start_button = digitalRead(START_BUTTON);
  if (start_button == ON && motorsOn == true) { // the motors are running and might need adjustment

    currentMillis = millis();
    duration = (currentMillis - startMillis);
    arm1_remainder = duration % (arm1_period * 1000);
    //arm2_remainder = duration % (arm2_period * 1000);
    //Serial.println("arm1 period: " + String(arm1_period) + ", arm2 period:  " + String(arm2_period));
    //Serial.println("Wave: " + String(WAVES[arm1_wave]) + ", arm1 period: " + String(arm1_period) + " duration: " + String(duration) + " arm1 remainder: " + String(arm1_remainder)  );

    if      (arm1_wave == WAVE_NONE)      arm1_adjustment = 0;
    else if (arm1_wave == WAVE_SQUARE)    arm1_adjustment = square_adjustment(    arm1_remainder, arm1_period, arm1_amp);
    else if (arm1_wave == WAVE_SAW)       arm1_adjustment = saw_adjustment(       arm1_remainder, arm1_period, arm1_amp);
    else if (arm1_wave == WAVE_TRIANGLE)  arm1_adjustment = triangle_adjustment(  arm1_remainder, arm1_period, arm1_amp);
    else if (arm1_wave == WAVE_SINE)      arm1_adjustment = sine_adjustment (     arm1_remainder, arm1_period, arm1_amp);

    if (arm1_adjustment != prev_arm1_adjustment) { // only change sopeeds when we need to
      new_Arm1Speed = Arm1Speed + arm1_adjustment;
      Serial.println("new arm1 speed: " + String(new_Arm1Speed)); // remove this after calibrtation
      if (new_Arm1Speed > MAX_SPEED)      analogWrite(ARM1, MAX_SPEED);
      else if (new_Arm1Speed < MIN_SPEED) analogWrite(ARM1, 0);
      else                                analogWrite(ARM1, new_Arm1Speed);
      prev_arm1_adjustment = arm1_adjustment;
      Serial.println("Arm1Speed: " + String(Arm1Speed) + " Adj arm speed: " + String(arm1_adjustment));
    }

    // print the time taken
    if (int(prevMillis / 1000) != int(currentMillis / 1000)) {
      Serial.println("prev: " + String(int(prevMillis / 1000)) + " current: " + String(int(currentMillis / 1000)));
      lcd_display(get_status(), get_wave_status());
      prevMillis = currentMillis;
    }

    /*
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
    */

    //Serial.println("Arm2Speed: " + String(Arm2Speed) + " Adj arm speed: " + String(arm2_adjustment));
  }

  /*
     Start the motors
  */

  if (start_button == ON && motorsOn == false) { // start the motors
    // Necessary? The starting voltages might be incorrect depending on waves function choices
    analogWrite(PLATTER, PlatterSpeed);
    analogWrite(ARM1, Arm1Speed);
    //analogWrite(ARM2, Arm2Speed);
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
      //analogWrite(ARM2, 0);
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

    /*
        if (button_9_state == ON) { // Arm2
          int advance_speed = int(Arm2Speed / 2);
          if (advance_speed < MIN_SPEED) advance_speed = MIN_SPEED;
          analogWrite(ARM2, advance_speed);
          while (digitalRead(BUTTON_9) == ON);
          analogWrite(ARM2, 0);
        }
    */

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
