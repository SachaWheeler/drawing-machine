// include the lcd code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 9, d5 = 8, d6 = 7, d7 = 6; // 2, 3, 4, 5 become 6, 7, 8, 9
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// motors
int Arm1Pin = 2;
int PlatterPin = 3;
int Arm2Pin = 4;

void setup() {
  // LCD
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");

  // motors
  pinMode(Arm1Pin, OUTPUT);
  pinMode(Arm2Pin, OUTPUT);
  pinMode(PlatterPin, OUTPUT);
  Serial.begin(9600);
  while (! Serial);
  Serial.println("Speed 0 to 255");

}

void loop() {
  if (Serial.available()) {
      int speed = Serial.parseInt();
      if (speed >= 0 && speed <= 255) {
         analogWrite(Arm1Pin, speed);
         delay(1000);
         analogWrite(Arm2Pin, speed);
         delay(1000);
         analogWrite(PlatterPin, speed);
      }
   }

  // LCD
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);

}
