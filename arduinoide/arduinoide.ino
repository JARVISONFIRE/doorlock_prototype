// Include Arduino Wire library for I2C
#include <Wire.h>
// Include LCD display library for I2C
#include <LiquidCrystal_I2C.h>
// Include Keypad library
#include <Keypad.h>

// Length of password + 1 for null character
#define Password_Length 9

// Character to hold password input
char Data[Password_Length];

// Password
char Master[Password_Length] = "12345678";

// Pin connected to lock relay input
int lockOutput = 11;

// counter for incorrect attempts, max att
int countinc = 0;
#define MAXINC 3

// Counter for character entries
byte data_count = 0;

// Character to hold key input
char customKey;

// Constants for row and column sizes
const byte ROWS = 4;
const byte COLS = 3;

// Array to represent keys on keypad
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

// Connections to Arduino
byte rowPins[ROWS] = { 8, 7, 6, 5 };
byte colPins[COLS] = { 4, 3, 2 };

// Create keypad object
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Create LCD object
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Setup LCD with backlight and initialize
  lcd.backlight();
  lcd.init();

  // Set lockOutput as an OUTPUT pin
  pinMode(lockOutput, OUTPUT);
}

void loop() {

  // Initialize LCD and print
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");

  // Look for keypress
  customKey = customKeypad.getKey();
  if (customKey) {

    // if *, show password
    if (customKey == '*') {
      showPass(0);
      goto shown;
    }

    // if #, backspace
    if (customKey == '#') {
      backspace(0);
      goto shown;
    }


    // Enter keypress into array and increment counter

    Data[data_count] = customKey;
    lcd.setCursor(data_count, 1);
    lcd.print('*');
    data_count++;
  }

  // See if we have reached the password length
  if (data_count == Password_Length - 1) {
    delay(500);
    lcd.clear();

    if (!strcmp("00000000", Data)) {
      changepass();
      lcd.clear();
      clearData();
      goto shown;
    }

    if (!strcmp(Data, Master)) {
      // Password is correct
      lcd.print("Correct");
      // Turn on relay for 5 seconds
      digitalWrite(lockOutput, HIGH);
      delay(5000);
      digitalWrite(lockOutput, LOW);
    } else {
      // Password is incorrect
      lcd.print("Incorrect");
      delay(1000);
      countinc++;
      if (countinc == MAXINC) {
        lcd.clear();
        lcd.print("Too many in-");
        lcd.setCursor(0, 1);
        lcd.print("correct attempts.");
        delay(2000);
        lcd.clear();
        lcd.print("Try again later");
        delay(7000);
        countinc = 0;
      }
    }

    // Clear data and LCD display
    lcd.clear();
    clearData();
  }

shown:
  NULL;
}

void clearData() {
  // Go through array and clear data
  // put inside bracket to be cool
  while (data_count != 0) {
    Data[data_count] = 0;
    data_count--;
  }
  return;
}

// argument mode
// 0: called from std input
// 1: called from reset old
// 2: called from reset new
void showPass(int mode) {
  // Clear screen, set cursor to start
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Check Password:");
  lcd.setCursor(0, 1);
  // print every char in password
  for (int i = 0; i < data_count; i++) {
    if (mode == 0 || mode == 1)
      lcd.print(Data[i]);
    else if (mode == 2)
      lcd.print(Master[i]);
  }
  // wait for one second
  delay(1500);
  // clear screen, set cursor to start
  lcd.clear();
  lcd.setCursor(0, 0);
  switch (mode) {
    case 0:
      lcd.print("Enter Password:");
      break;
    case 1:
      lcd.print("Enter Old Passwd:");
      break;
    case 2:
      lcd.print("Enter New Passwd:");
      break;
  }
  lcd.setCursor(0, 1);
  // print stars
  for (int i = 0; i < data_count; i++) {
    lcd.print('*');
  }
}

// argument mode
// 0: called from std input
// 1: called from reset old
// 2: called from reset new
void backspace(int mode) {
  if (data_count) {
    data_count--;
    lcd.clear();
    switch (mode) {
      case 0:
        lcd.print("Enter Password:");
        break;
      case 1:
        lcd.print("Enter Old Passwd:");
        break;
      case 2:
        lcd.print("Enter New Passwd:");
        break;
    }
    lcd.setCursor(0, 1);
    for (int i = 0; i < data_count; i++) {
      lcd.print('*');
    }
  }
}

void changepass() {
  // clear screen, ask for confirmation
  clearData();
  lcd.clear();
  lcd.print("Reset Password?");
  lcd.setCursor(0, 1);
  lcd.print("Yes (1), No (0)");

  // read keys until press
  do {
    customKey = customKeypad.getKey();
  } while (!customKey);

  lcd.clear();

  switch (customKey) {
    case '1':  // agreed
      lcd.print("Enter Old Passwd:");

      while (data_count != Password_Length - 1) {
        // read keys until press
        do {
          customKey = customKeypad.getKey();
        } while (!customKey);

        // show password
        if (customKey == '*') {
          showPass(1);
          continue;
        }

        // backspace
        if (customKey == '#') {
          backspace(1);
          continue;
        }

        // Enter keypress into array and increment counter
        Data[data_count] = customKey;
        lcd.setCursor(data_count, 1);
        lcd.print('*');
        data_count++;
      }

      lcd.clear();

      // check if it matches
      if (!strcmp(Data, Master)) {  // matches
        lcd.print("Correct");
        delay(2000);

        // jump for zeroes stuff
        newmaster:
          NULL;

        lcd.clear();
        lcd.print("Enter New Passwd:");

        // get new passwd
        data_count = 0;
        while (data_count != Password_Length - 1) {
          // read keys until press
          do {
            customKey = customKeypad.getKey();
          } while (!customKey);

          // show password
          if (customKey == '*') {
            showPass(2);
            continue;
          }

          // backspace
          if (customKey == '#') {
            backspace(2);
            continue;
          }

          // Enter keypress into array and increment counter
          Master[data_count] = customKey;
          lcd.setCursor(data_count, 1);
          lcd.print('*');
          data_count++;
        }

        delay(1000);
        lcd.clear();

        // check if pass is zeroes
        if (!strcmp(Master, "00000000")) {
          lcd.print("Invalid");
          lcd.setCursor(0, 1);
          lcd.print("Password");
          delay(2000);
          lcd.clear();
          lcd.print("Password cannot");
          lcd.setCursor(0, 1);
          lcd.print("be 00000000");
          delay(2000);
          lcd.clear();
          lcd.print("Try again");
          delay(2000);
          goto newmaster;
        }


        lcd.print("Password Reset");
        lcd.setCursor(0, 1);
        lcd.print("Successfully");
        delay(2500);
      } else {  // doesn't match
        lcd.print("Incorrect");
        delay(2000);
      }

      return;
      break;
    case '0':  // declined
      lcd.print("Reset cancelled");
      delay(2000);
      return;
      break;
    default:  // invalid
      lcd.print("Invalid input");
      delay(2000);
      changepass();
      return;
      break;
  }
}
