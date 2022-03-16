// include the lcd code:
#include <LiquidCrystal.h>

// LCD variables - https://www.arduino.cc/en/Reference/LiquidCrystalConstructor
const int rs = 12, en = 11, d4 = 9, d5 = 8, d6 = 7, d7 = 6; // 2, 3, 4, 5 become 6, 7, 8, 9
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// motors
const int Arm1Pin = 2;
const int PlatterPin = 3;
const int Arm2Pin = 4;

// control panel
const int START = 22;     // the number of the pushbutton pin
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
int PlatterSpeed = 100; // initial variables, will change
int Arm1Speed = 100;
int Arm2Speed = 100;
const int max_platter_speed = 255;
const int max_arm1_speed = 255;
const int max_arm2_speed = 255;

bool motorsOn = false;
int start;

int button_1_state = 0;
int button_2_state = 0;
int button_3_state = 0;
int button_4_state = 0;
int button_5_state = 0;
int button_6_state = 0;
int button_7_state = 0;
int button_8_state = 0;
int button_9_state = 0;

String status;

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 100;  //the value is a number of milliseconds

void setup() {
  // LCD
  lcd.begin(16, 2);
  // Print a message to the LCD.
  status = "P:" + String(PlatterSpeed) + ",A:" + Arm1Speed + ",B:" + Arm2Speed;
  lcd.setCursor(0, 0);
  lcd.print(status);

  // buttons
  pinMode(START, INPUT);
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  pinMode(BUTTON_3, INPUT);
  pinMode(BUTTON_4, INPUT);
  pinMode(BUTTON_5, INPUT);
  pinMode(BUTTON_6, INPUT);
  pinMode(BUTTON_7, INPUT);
  pinMode(BUTTON_8, INPUT);
  pinMode(BUTTON_9, INPUT);

  // motors
  pinMode(Arm1Pin, OUTPUT);
  pinMode(Arm2Pin, OUTPUT);
  pinMode(PlatterPin, OUTPUT);
  Serial.begin(9600);

  startMillis = millis();  //initial start time

  // serial input deprecated
  // while (! Serial);
  // Serial.println("Speed 0 to 255");
}

void loop() {

  start = digitalRead(START);

  // Serial.print("start: ");
  // Serial.println(start);
  if (start == HIGH && motorsOn == false) { // run the motors
    Serial.println("run");

    // start them
    // analogWrite(Arm1Pin, Arm1Speed);
    // analogWrite(Arm2Pin, Arm2Speed);
    // analogWrite(PlatterPin, PlatterSpeed);
    motorsOn = true;
    Serial.println("motors started");

  }

  if (start == LOW) {
    Serial.println("stop");
    if (motorsOn == true) { // stop the motors
      // analogWrite(PlatterPin, 0);
      // analogWrite(Arm1Pin, 0);
      // analogWrite(Arm2Pin, 0);
      motorsOn = false;
      Serial.println("motors stopped");
    }
    // set the speeds
    button_1_state = digitalRead(BUTTON_1);
    button_2_state = digitalRead(BUTTON_2);
    button_3_state = digitalRead(BUTTON_3);
    button_4_state = digitalRead(BUTTON_4);
    button_5_state = digitalRead(BUTTON_5);
    button_6_state = digitalRead(BUTTON_6);
    button_7_state = digitalRead(BUTTON_7);
    button_8_state = digitalRead(BUTTON_8);
    button_9_state = digitalRead(BUTTON_9);
    //Serial.print("buttons:");
    //Serial.print(button_1_state);
    //Serial.print(button_2_state);
    //Serial.print(button_4_state);
    //Serial.print(button_5_state);
    //Serial.print(button_7_state);
    //Serial.println(button_8_state);
    String buttons = String(start) + ":" + String(button_1_state) + " " + String(button_2_state) + " " + String(button_4_state) + " " + String(button_5_state) + " " + String(button_7_state) + " " + String(button_8_state);

    if (button_1_state == HIGH && PlatterSpeed < max_platter_speed){ // platter speed up
      PlatterSpeed += 1;
      button_1_state = LOW;
    }
    if (button_2_state == HIGH && PlatterSpeed > 0){ // platter speed down
      PlatterSpeed -= 1;
      button_2_state = LOW;
    }

    if (button_4_state == HIGH && Arm1Speed < max_arm1_speed){ // Arm1 speed up
      Arm1Speed += 1;
      button_4_state = LOW;
    }
    if (button_5_state == HIGH && Arm1Speed > 0){ // Arm1Speed speed down
      Arm1Speed -= 1;
      button_5_state = LOW;
    }

    if (button_7_state == HIGH && Arm2Speed < max_arm2_speed){ // Arm2 speed up
      Arm2Speed += 1;
      button_7_state = LOW;
    }
    if (button_8_state == HIGH && Arm2Speed > 0){ // Arm2 speed down
      Arm2Speed -= 1;
      button_8_state = LOW;
    }

    status = "P:" + String(PlatterSpeed) + ",A:" + Arm1Speed + ",B:" + Arm2Speed + "   ";
    lcd.setCursor(0, 0);
    lcd.print(status);
    lcd.setCursor(0, 1);
    lcd.print(buttons);
  }
}

// LCD
// set the cursor to column 0, line 1
// (note: line 1 is the second row, since counting begins with 0):
//lcd.setCursor(0, 1);
// print the number of seconds since reset:
// lcd.print(millis() / 1000);
