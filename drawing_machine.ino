// include the lcd code:
#include <LiquidCrystal.h>

// LCD variables - https://www.arduino.cc/en/Reference/LiquidCrystalConstructor
const int rs = 12, en = 11, d4 = 9, d5 = 8, d6 = 7, d7 = 6; // 2, 3, 4, 5 become 6, 7, 8, 9
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// motors
const int Arm2Pin    = 2;
const int PlatterPin = 3;
const int Arm1Pin    = 4;

// control panel 1
const int START_BUTTON = 34; // start button

const int BUTTON_1 = 36;     // row 1 col 1
const int BUTTON_2 = 38;     // row 2 col 1
const int BUTTON_3 = 40;     // row 3 col 1
const int BUTTON_4 = 42;     // row 1 col 2
const int BUTTON_5 = 44;     // row 2 col 2
const int BUTTON_6 = 46;     // row 3 col 2
const int BUTTON_7 = 48;     // row 1 col 3
const int BUTTON_8 = 50;     // row 2 col 3
const int BUTTON_9 = 52;     // row 3 col 3

// control panel 2
const int SELECTOR_A_1 = 53; // first selector, first option
const int SELECTOR_A_2 = 51;
const int SELECTOR_A_3 = 49;
const int SELECTOR_A_4 = 47;
const int SELECTOR_A_5 = 45;
const int SELECTOR_B_1 = 43;
const int SELECTOR_B_2 = 41;
const int SELECTOR_B_3 = 39;
const int SELECTOR_B_4 = 37;
const int SELECTOR_B_5 = 35;

const int BUTTON_A_FREQ_UP = 22;
const int BUTTON_A_FREQ_DOWN = 24;
const int BUTTON_A_PERIOD_UP = 26;
const int BUTTON_A_PERIOD_DOWN = 28;
const int BUTTON_B_FREQ_UP = 23;
const int BUTTON_B_FREQ_DOWN = 25;
const int BUTTON_B_PERIOD_UP = 27;
const int BUTTON_B_PERIOD_DOWN = 29;

int button_a_freq_up = 0, button_a_freq_down = 0, button_a_period_up = 0, button_a_period_down = 0, button_b_freq_up = 0, button_b_freq_down = 0, button_b_period_up = 0, button_b_period_down = 0;
int selector_a_1 = 0, selector_a_2 = 0, selector_a_3 = 0, selector_a_4 = 0, selector_a_5 = 0, selector_b_1 = 0, selector_b_2 = 0, selector_b_3 = 0, selector_b_4 = 0, selector_b_5 = 0;
long selectorMillis = 0;
const long SELECTOR_INTERVAL = 500;

const int BUTTON_PAUSE = 100; // milliseconds to wait while button being pressed

// variables
int PlatterSpeed = 60; // initial variables, will change
int Arm1Speed    = 150;
int Arm2Speed    = 150;

unsigned int adjusted_platter_speed;
unsigned int adjusted_arm1_speed;
unsigned int adjusted_arm2_speed;
unsigned int duration;
unsigned int remainder;
float slope;
unsigned int adjustment;

const int MAX_PLATTER_SPEED = 255;
const int MAX_ARM1_SPEED    = 255;
const int MAX_ARM2_SPEED    = 255;
const int MOTOR_MIN         = 55;

bool motorsOn = false;
int start_button;

int button_1_state = 0, button_2_state = 0, button_3_state = 0, button_4_state = 0, button_5_state = 0, button_6_state = 0, button_7_state = 0, button_8_state = 0, button_9_state = 0;
bool button_change = false;

const char *Waves[] = {"None", "Square", "Saw",  "Triangle", "Sine"};

// int arm1_wave = 0, arm2_wave = 0, arm1_period = 1, arm2_period = 1, arm1_amp = 1, arm2_amp = 1;
unsigned int arm1_wave = 0, arm1_period = 1, arm1_amp = 1; // testing
String wave_status; // testing

String lcd_status;

unsigned long startMillis, currentMillis;
const unsigned long PERIOD = 100;  // milliseconds

const bool ON = LOW, OFF = HIGH;
const bool RUN_MOTORS = true; // change this to false for testing without motors

String get_status(int Arm1Speed, int PlatterSpeed, int Arm2Speed) {
  return "A" + String(Arm1Speed) + " B" + PlatterSpeed + " C" + Arm2Speed + "   ";
}

void setup() {
  // LCD
  lcd.begin(16, 2);
  lcd_status = get_status(Arm1Speed, PlatterSpeed, Arm2Speed);

  // buttons
  pinMode(START_BUTTON, INPUT_PULLUP);

  // pause startup if START_BUTTON is pushed
  start_button = digitalRead(START_BUTTON);
  if (start_button == ON) {
    lcd.setCursor(0, 0);
    lcd.print("Start button");
    lcd.setCursor(0, 1);
    lcd.print("must be OFF!");
    while (digitalRead(START_BUTTON) == ON); // wait until turned off
    lcd.clear();
  }
  lcd.setCursor(0, 0);
  lcd.print("Starting...");

  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);
  pinMode(BUTTON_5, INPUT_PULLUP);
  pinMode(BUTTON_6, INPUT_PULLUP);
  pinMode(BUTTON_7, INPUT_PULLUP);
  pinMode(BUTTON_8, INPUT_PULLUP);
  pinMode(BUTTON_9, INPUT_PULLUP);

  pinMode(SELECTOR_A_1, INPUT_PULLUP);
  pinMode(SELECTOR_A_2, INPUT_PULLUP);
  pinMode(SELECTOR_A_3, INPUT_PULLUP);
  pinMode(SELECTOR_A_4, INPUT_PULLUP);
  pinMode(SELECTOR_A_5, INPUT_PULLUP);
  pinMode(SELECTOR_B_1, INPUT_PULLUP);
  pinMode(SELECTOR_B_2, INPUT_PULLUP);
  pinMode(SELECTOR_B_3, INPUT_PULLUP);
  pinMode(SELECTOR_B_4, INPUT_PULLUP);
  pinMode(SELECTOR_B_5, INPUT_PULLUP);

  pinMode(BUTTON_A_FREQ_UP,     INPUT_PULLUP);
  pinMode(BUTTON_A_FREQ_DOWN,   INPUT_PULLUP);
  pinMode(BUTTON_A_PERIOD_UP,   INPUT_PULLUP);
  pinMode(BUTTON_A_PERIOD_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_B_FREQ_UP,     INPUT_PULLUP);
  pinMode(BUTTON_B_FREQ_DOWN,   INPUT_PULLUP);
  pinMode(BUTTON_B_PERIOD_UP,   INPUT_PULLUP);
  pinMode(BUTTON_B_PERIOD_DOWN, INPUT_PULLUP);

  // motors
  pinMode(Arm1Pin, OUTPUT);
  pinMode(Arm2Pin, OUTPUT);
  pinMode(PlatterPin, OUTPUT);

  Serial.begin(9600);
  // startMillis = millis();  //initial start time

  lcd.setCursor(0, 0);
  lcd.print(lcd_status);
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

    // testing vars
    arm1_wave = 3;
    arm1_period = 500; //milliseconds
    arm1_amp = 40;

    currentMillis = millis();
    duration = int(currentMillis - startMillis);
    remainder = duration % (arm1_period);

    wave_status = "Arm1: speed:" + String(Arm1Speed) + " wave:" + String(Waves[arm1_wave]) + " period:" + String(arm1_period) + " amp:" + String(arm1_amp);
    //Serial.println(wave_status);
    Serial.println("remainder: " + String(remainder));

    if (arm1_wave == 0) { // no wave

      adjusted_arm1_speed = Arm1Speed;

    } else if (arm1_wave == 1) { // square - DONE

      if (remainder <= int(arm1_period / 2)) { // HIGH
        adjustment = int(arm1_amp / 2);
      } else { // LOW
        adjustment = -int(arm1_amp / 2);
      }
      adjusted_arm1_speed = Arm1Speed + adjustment;

    } else if (arm1_wave == 2) { // saw DONE

      // ramps up, then snaps back
      slope = 1000 * arm1_amp / arm1_period;
      //Serial.println(slope);
      adjustment = -int(arm1_amp / 2) + int(slope * remainder / 1000);
      adjusted_arm1_speed = Arm1Speed + adjustment;

    } else if (arm1_wave == 3) { // triangle


      /*
              |   /\
              |__/__\___
              | /    \
              |/______\_________
      */


      // ramps up for half the period, then ramps back down
      slope = 2 * 1000 * arm1_amp / arm1_period; // half the period so twice the slope
      if (remainder <= int(arm1_period / 2)) { // ASCENDING
        Serial.println("up");
        adjustment = -int(arm1_amp / 2) + int(slope * remainder / 1000);
        Serial.println(remainder);
      } else { // DESCENDING
        Serial.println("down");
        adjustment = -int(arm1_amp / 2) + int(arm1_amp - slope * arm1_period / 1000);
        Serial.println(arm1_period - remainder);
      }
      adjusted_arm1_speed = Arm1Speed + adjustment;



    } else if (arm1_wave == 4) { // sine

      adjusted_arm1_speed = Arm1Speed;

    }
    Serial.println("ArmSpeed: " + String(Arm1Speed) + " Adj arm speed: " + String(adjusted_arm1_speed));
  }

  if (start_button == ON && motorsOn == false) { // start the motors
    Serial.println("run");

    if (RUN_MOTORS == true) { // start them
      analogWrite(Arm1Pin, Arm1Speed);
      analogWrite(Arm2Pin, Arm2Speed);
      analogWrite(PlatterPin, PlatterSpeed);
      Serial.println("motors started");
    }
    motorsOn = true;
    startMillis = millis();  //initial start time
  }

  if (start_button == OFF) {
    if (motorsOn == true) { // stop the motors
      Serial.println("stop");
      if (RUN_MOTORS == true) {
        analogWrite(PlatterPin, 0);
        analogWrite(Arm1Pin, 0);
        analogWrite(Arm2Pin, 0);
        Serial.println("motors stopped");
      }
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

    if (button_1_state == ON && Arm1Speed < MAX_ARM1_SPEED) { // Arm1 speed up
      Arm1Speed += 1;
      button_change = true;
      delay(BUTTON_PAUSE);
    }
    if (button_2_state == ON && Arm1Speed > MOTOR_MIN) { // Arm1 speed down
      Arm1Speed -= 1;
      button_change = true;
      delay(BUTTON_PAUSE);
    }

    if (button_4_state == ON && PlatterSpeed < MAX_PLATTER_SPEED) { // Platter speed up
      PlatterSpeed += 1;
      button_change = true;
      delay(BUTTON_PAUSE);
    }
    if (button_5_state == ON && PlatterSpeed > MOTOR_MIN) { // Platter speed down
      PlatterSpeed -= 1;
      button_change = true;
      delay(BUTTON_PAUSE);
    }

    if (button_7_state == ON && Arm2Speed < MAX_ARM2_SPEED) { // Arm2 speed up
      Arm2Speed += 1;
      button_change = true;
      delay(BUTTON_PAUSE);
    }
    if (button_8_state == ON && Arm2Speed > MOTOR_MIN) { // Arm2 speed down
      Arm2Speed -= 1;
      button_change = true;
      delay(BUTTON_PAUSE);
    }

    // advance Arms
    if (button_3_state == ON) { // Arm1
      int advance_speed = int(Arm1Speed / 2);
      if (advance_speed < MOTOR_MIN) advance_speed = MOTOR_MIN;
      analogWrite(Arm1Pin, advance_speed);
      while (digitalRead(BUTTON_3) == ON) { // advance Arm1
        delay(50);
      }
      analogWrite(Arm1Pin, 0);
    }

    if (button_9_state == ON) { // Arm2
      int advance_speed = int(Arm2Speed / 2);
      if (advance_speed < MOTOR_MIN) advance_speed = MOTOR_MIN;
      analogWrite(Arm2Pin, advance_speed);
      while (digitalRead(BUTTON_9) == ON) { // advance Arm2
        delay(50);
      }
      analogWrite(Arm2Pin, 0);
    }

    if (currentMillis - selectorMillis >= SELECTOR_INTERVAL) {
      // only run every SELECTOR_INTERVAL (500) millis
      selectorMillis = currentMillis;

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

      /*
            These are giving a false ON (LOW) signal. test connection


        08:21:16.717 -> selector_a_4 //  a4 is giving false positives (ON, LOW

      */
      if (selector_a_1 == ON) {
        Serial.println("selector_a_1");

      }
      if (selector_a_2 == ON) {
        Serial.println("selector_a_2");
        delay(BUTTON_PAUSE);
      }
      if (selector_a_3 == ON) {
        Serial.println("selector_a_3");
        delay(BUTTON_PAUSE);
      }
      if (selector_a_4 == ON) { // test connection
        Serial.println("selector_a_4");
        delay(BUTTON_PAUSE);
      }
      if (selector_a_5 == ON) {
        Serial.println("selector_a_5");
        delay(BUTTON_PAUSE);
      }
      if (selector_b_1 == ON) {
        Serial.println("selector_b_1");
        delay(BUTTON_PAUSE);
      }
      if (selector_b_2 == ON) {
        Serial.println("selector_b_2");
        delay(BUTTON_PAUSE);
      }
      if (selector_b_3 == ON) {
        Serial.println("selector_b_3");
        delay(BUTTON_PAUSE);
      }
      if (selector_b_4 == ON) {
        Serial.println("selector_b_4");
        delay(BUTTON_PAUSE);
      }
      if (selector_b_5 == ON) {
        Serial.println("selector_b_5");
        delay(BUTTON_PAUSE);
      }
      Serial.println(" ");
      /*   */
    }

    button_a_freq_up     = digitalRead(BUTTON_A_FREQ_UP);
    button_a_freq_down   = digitalRead(BUTTON_A_FREQ_DOWN);
    button_a_period_up   = digitalRead(BUTTON_A_PERIOD_UP);
    button_a_period_down = digitalRead(BUTTON_A_PERIOD_DOWN);
    button_b_freq_up     = digitalRead(BUTTON_B_FREQ_UP);
    button_b_freq_down   = digitalRead(BUTTON_B_FREQ_DOWN);
    button_b_period_up   = digitalRead(BUTTON_B_PERIOD_UP);
    button_b_period_down = digitalRead(BUTTON_B_PERIOD_DOWN);

    if (button_a_freq_up == ON) {
      Serial.println("button_a_freq_up");
    }
    if (button_a_freq_down == ON) {
      Serial.println("button_a_freq_down");
    }
    if (button_a_period_up == ON) {
      Serial.println("button_a_period_up");
    }
    if (button_a_period_down == ON) {
      Serial.println("button_a_period_down");
    }
    if (button_b_freq_up == ON) {
      Serial.println("button_b_freq_up");
    }
    if (button_b_freq_down == ON) {
      Serial.println("button_b_freq_down");
    }
    if (button_b_period_up == ON) {
      Serial.println("button_b_period_up");
    }
    if (button_b_period_down == ON) {
      Serial.println("button_b_period_down");
    }

    // change LCD status
    if (button_change == true) {
      lcd_status = get_status(Arm1Speed, PlatterSpeed, Arm2Speed);
      lcd.setCursor(0, 0);
      lcd.print(lcd_status);
      delay(100);
      button_change = false;
    }
  }
}
