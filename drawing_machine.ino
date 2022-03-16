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
const int max_platter_speed = 255;
const int max_arm1_speed = 255;
const int max_arm2_speed = 255;
const int motor_min = 50;

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

/*
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 100;  // milliseconds
*/

const bool ON = LOW;
const bool OFF = HIGH;

const bool RUN_MOTORS = true; // change this to false for testing without motors

void setup() {
  // LCD
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd_status = "P:" + String(PlatterSpeed) + ",A:" + Arm1Speed + ",B:" + Arm2Speed;
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

  // startMillis = millis();  //initial start time
}

void loop() {

  start_button = digitalRead(START_BUTTON);

  if (start_button == ON && motorsOn == false) { // run the motors
    Serial.println("run");

    if (RUN_MOTORS == true) { // start them
      analogWrite(Arm1Pin, Arm1Speed);
      analogWrite(Arm2Pin, Arm2Speed);
      analogWrite(PlatterPin, PlatterSpeed);
      Serial.println("motors started");
    }
    motorsOn = true;
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

    if (button_1_state == ON && PlatterSpeed < max_platter_speed) { // platter speed up
      PlatterSpeed += 1;
      button_change = true;
    }
    if (button_2_state == ON && PlatterSpeed > motor_min) { // platter speed down
      PlatterSpeed -= 1;
      button_change = true;
    }

    if (button_4_state == ON && Arm1Speed < max_arm1_speed) { // Arm1 speed up
      Arm1Speed += 1;
      button_change = true;
    }
    if (button_5_state == ON && Arm1Speed > motor_min) { // Arm1Speed speed down
      Arm1Speed -= 1;
      button_change = true;
    }

    if (button_7_state == ON && Arm2Speed < max_arm2_speed) { // Arm2 speed up
      Arm2Speed += 1;
      button_change = true;
    }
    if (button_8_state == ON && Arm2Speed > motor_min) { // Arm2 speed down
      Arm2Speed -= 1;
      button_change = true;
    }

    if (button_change == true) {
      lcd_status = "P:" + String(PlatterSpeed) + " A:" + Arm1Speed + ",B:" + Arm2Speed + "   ";
      lcd.setCursor(0, 0);
      lcd.print(lcd_status);
      delay(100);
      button_change = false;
    }
  }
}
