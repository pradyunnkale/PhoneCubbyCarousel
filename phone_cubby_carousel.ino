//Changes are that there are now incorporated free slots, and the motor is now fixed and working as intended




#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#define MAX_SLOTS 30  // Number of slots to store passwords
const int STEPS_PER_SLOT = 6400/MAX_SLOTS;  // Number of steps per slot (adjust as needed)
// Define the keypad layout
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
 {'1', '2', '3', 'A'},
 {'4', '5', '6', 'B'},
 {'7', '8', '9', 'C'},
 {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
// LCD Setup (I2C address = 0x27, 16x2 screen)
LiquidCrystal_I2C lcd(0x27, 16, 2);
// Password storage array (for up to 10 passwords)
String userSlots[MAX_SLOTS];
String passwordSlots[MAX_SLOTS];
// Stepper motor driver pins
int stepPin = 13;  // Connect to STEP pin of A4968
int dirPin = 12;   // Connect to DIR pin of A4968
int enPin = 10;    // Connect to ENABLE pin of A4968 (Low to enable, High to disable)
// Track the current position of the motor
int currentSlot = 0;  // Assume the motor starts at slot 0
bool firstStore = true;
bool storePressed = true;
int freeSlot = -1;
String enteredUser = "init";


void setup() {
 Serial.begin(9600);
 lcd.init();
 lcd.backlight();
 // Initialize stepper motor driver pins
 pinMode(stepPin, OUTPUT);
 pinMode(dirPin, OUTPUT);
 pinMode(enPin, HIGH);
 // Enable the motor driver (set EN pin low)
 digitalWrite(enPin, LOW);
}


void loop() {
 char key = keypad.getKey();
 while (storePressed) {
  storePhone();
  if (!storePressed) {
    break;
  }
 }
 while (!storePressed) {
  retrievePhone();
  if (storePressed) {
    break;
  }
 }
}


void storePhone(){
  lcd.clear();
  lcd.print("NEW USER: * TO SWITCH");
  String storedUser = enterUser();
  // Free slot control so no one can take someone elses phone
  for (int i = 0; i < MAX_SLOTS; i++) {
    if (userSlots[i] == "") {
      freeSlot = i;  // Find the first empty slot
      break;
    }
  }
  if (storedUser != ""){
    if (freeSlot != -1) {
      userSlots[freeSlot] = storedUser;
      storePassword();
    } else {
      lcd.clear();
      lcd.print("ALL SLOTS FULL");
      delay(0);
    }
  }
}


void retrievePhone(){
  lcd.clear();
  lcd.print("ENTER USER: * TO SWITCH");
  String enteredUser = enterUser();
  int foundSlot = -1;
  bool found = false;
  for (int i = 0; i < MAX_SLOTS; i++){
    if (userSlots[i] == enteredUser){
      foundSlot = i;
      found = true;
      break;
    }
  }
  if (enteredUser != ""){
    if (found){
      lcd.clear();
      lcd.print("ENTER PSWD: ");
      String enteredPassword = enterPassword();
      if (enteredPassword == passwordSlots[foundSlot]){
        lcd.clear();
        lcd.print("CORRECT");
        moveMotorToSlot(foundSlot);  // Move motor to the correct slot
        userSlots[foundSlot] = "";
        passwordSlots[foundSlot] = "";  // Clear the password from the slot and makes it free
        delay(1000);
      } else {
        lcd.clear();
        lcd.print("INVALID PASSWORD");
        delay(1000);
      }
    } else {
      lcd.clear();
      lcd.print("USER NOT FOUND");
      delay(1000);
    }
  }
}


void storePassword() {
 lcd.clear();
 lcd.print("NEW PSWD:");
 // Wait for the user to enter a password
 String enteredPassword = enterPassword();
 // Store the password in the first available slot & go to that slot
 if (enteredPassword != ""){
   passwordSlots[freeSlot] = enteredPassword;
   lcd.clear();
   lcd.print("STORED");
   firstStore = false;
 //Finds next free slot and moves there
 for (int i = 0; i < MAX_SLOTS; i++) {
   if (passwordSlots[i] == "") {
     freeSlot = i;  // Find the first empty slot
     delay(500);
     moveMotorToSlot(freeSlot);
     delay(500);
     break;
   }
 }
 }
}






// Function to handle motor movement to the selected slot
void moveMotorToSlot(int targetSlot) {
 int stepsToTarget = (targetSlot - currentSlot) * STEPS_PER_SLOT; // Calculate steps to move to target slot
 // Set direction for the motor movement
 if (stepsToTarget > 0) {
   digitalWrite(dirPin, HIGH); // Forward movement
 } else {
   digitalWrite(dirPin, LOW);  // Backward movement
 }
stepsToTarget = abs(stepsToTarget);  // Ensure positive step count
 // Move motor to the target slot
 for (int i = 0; i < stepsToTarget; i++) {
   digitalWrite(stepPin, HIGH);
   delayMicroseconds(500);  // Control motor speed
   digitalWrite(stepPin, LOW);
   delayMicroseconds(500);  // Control motor speed
 }
 // Update the current slot to the new position
 currentSlot = targetSlot;
}


// Helper function to handle password input
String enterUser() {
 String enteredUser = "";
 while (true) {
   char key = keypad.getKey();
   if (key) {
    if (key == '*'){
      enteredUser = "";
      storePressed = !storePressed;
      break;
    }
     if (key == '#') {  // End input when '#' is pressed
       break;
     } else {
       enteredUser += key;  // Add key to password
       lcd.setCursor(0, 1);
       lcd.print(enteredUser);  // Display the password as it's entered
     }
   }
 }
 return enteredUser;
}


// Helper function to handle password input
String enterPassword() {
 String enteredPassword = "";
 while (true) {
   char key = keypad.getKey();
   if (key) {
    if (key == '*'){
      enteredPassword = "";
      storePressed = !storePressed;
      break;
    }
     if (key == '#') {  // End input when '#' is pressed
       break;
     } else {
       enteredPassword += key;  // Add key to password
       lcd.setCursor(0, 1);
       lcd.print(enteredPassword);  // Display the password as it's entered
     }
   }
 }
 return enteredPassword;
}
