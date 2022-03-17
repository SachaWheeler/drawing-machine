// include the lcd code:
#include <LiquidCrystal.h>

// LCD variables - https://www.arduino.cc/en/Reference/LiquidCrystalConstructor
const int rs = 12, en = 11, d4 = 9, d5 = 8, d6 = 7, d7 = 6; // 2, 3, 4, 5 become 6, 7, 8, 9
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// motors
const int Arm2Pin = 2;
const int PlatterPin = 3;
const int Arm1Pin = 4;

// control panel
const int START_BUTTON = 22; // the start button
const int BUTTON_1 = 36;   // row 1 col 1
const int BUTTON_2 = 38;   // row 2 col 1
const int BUTTON_3 = 40;   // row 3 col 1
const int BUTTON_4 = 42;   // row 1 col 2
const int BUTTON_5 = 44;   // row 2 col 2
const int BUTTON_6 = 46;   // row 3 col 2
const int BUTTON_7 = 48;   // row 1 col 3
const int BUTTON_8 = 50;   // row 2 col 3
const int BUTTON_9 = 52;   // row 3 col 3

// variables
int PlatterSpeed = 60; // initial variables, will change
int Arm1Speed = 100;
int Arm2Speed = 100;
const int MAX_PLATTER_SPEED = 255;
const int MAX_ARM1_SPEED = 255;
const int MAX_ARM2_SPEED = 255;
const int MOTOR_MIN = 55;

bool motorsOn = false;
int start_button;

int button_1_state = 0;
int button_2_state = 0;
int button_3_state = 0;
int button_4_state = 0;
int button_5_state = 0;
int button_6_state = 0;
int button_7_state = 0;
int button_8_state = 0;
int button_9_state = 0;

bool button_change = false;

String lcd_status;

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long PERIOD = 100;  // milliseconds

const bool ON = LOW;
const bool OFF = HIGH;

const bool RUN_MOTORS = true; // change this to false for testing without motors

String get_status(int Arm1Speed, int PlatterSpeed, int Arm2Speed) {
  return "A" + String(Arm1Speed) + " B" + PlatterSpeed + " C" + Arm2Speed + "   ";
}

void setup() {
  // LCD
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd_status = get_status(Arm1Speed, PlatterSpeed, Arm2Speed);
  lcd.setCursor(0, 0);
  lcd.print(lcd_status);

  // buttons
  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUTTON_3, INPUT_PULLUP);
  pinMode(BUTTON_4, INPUT_PULLUP);
  pinMode(BUTTON_5, INPUT_PULLUP);
  pinMode(BUTTON_6, INPUT_PULLUP);
  pinMode(BUTTON_7, INPUT_PULLUP);
  pinMode(BUTTON_8, INPUT_PULLUP);
  pinMode(BUTTON_9, INPUT_PULLUP);

  // motors
  pinMode(Arm1Pin, OUTPUT);
  pinMode(Arm2Pin, OUTPUT);
  pinMode(PlatterPin, OUTPUT);
  Serial.begin(9600);

  startMillis = millis();  //initial start time
}

void loop() {

  start_button = digitalRead(START_BUTTON);
  if (start_button == ON && motorsOn == true) {
    // the motors are running and might need adjustment
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

    if (button_1_state == ON && Arm1Speed < MAX_ARM1_SPEED) { // platter speed up
      Arm1Speed += 1;
      button_change = true;
    }
    if (button_2_state == ON && Arm1Speed > MOTOR_MIN) { // platter speed down
      Arm1Speed -= 1;
      button_change = true;
    }

    if (button_4_state == ON && PlatterSpeed < MAX_PLATTER_SPEED) { // Arm1 speed up
      PlatterSpeed += 1;
      button_change = true;
    }
    if (button_5_state == ON && PlatterSpeed > MOTOR_MIN) { // Arm1Speed speed down
      PlatterSpeed -= 1;
      button_change = true;
    }

    if (button_7_state == ON && Arm2Speed < MAX_ARM2_SPEED) { // Arm2 speed up
      Arm2Speed += 1;
      button_change = true;
    }
    if (button_8_state == ON && Arm2Speed > MOTOR_MIN) { // Arm2 speed down
      Arm2Speed -= 1;
      button_change = true;
    }

    // advance Arms
    if (button_3_state == ON) {
      int advance_speed = int(Arm1Speed / 2);
      if (advance_speed < MOTOR_MIN) advance_speed = MOTOR_MIN;
      analogWrite(Arm1Pin, advance_speed);
      while (digitalRead(BUTTON_3) == ON) { // advance Arm1
        delay(50);
      }
      analogWrite(Arm1Pin, 0);
    }

    if (button_9_state == ON) {
      int advance_speed = int(Arm1Speed / 2);
      if (advance_speed < MOTOR_MIN) advance_speed = MOTOR_MIN;
      analogWrite(Arm2Pin, advance_speed);
      while (digitalRead(BUTTON_9) == ON) { // advance Arm2
        delay(50);
      }
      analogWrite(Arm2Pin, 0);
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
