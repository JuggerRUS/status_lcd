//
// Change log:
// 
// 
// 
// 
// 
//
#define VERSION "    Ver. 0.0    "
#define COMREADTIMEOUT 20

// include the library code:
#include <LiquidCrystal.h>
#include <EEPROM.h>

String input;
byte size;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 4, 5, 6, 7);

const int contrastPIN = 3;
const byte interruptPin = 2;
const byte contrastLVL_min = 20;
const byte contrastLVL_max = 120;
const byte contrastLVL_step = 5;
const byte contrastLVL_default = 65;
byte contrastLVL;
const int contrastLVL_address = 0;
bool set_contrast_mode = false;
volatile unsigned long contrast_mills;

void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");
}

void set_contrast() {
  attachInterrupt(digitalPinToInterrupt(interruptPin), btnPress_contrast, FALLING);
  contrast_mills = millis();
  contrastLVL -= contrastLVL_step; // Нивелируем дополнительный клик, вызванный удерживанием кнопки для входа в этот режим (лишний раз срабатывает прерывание по кнопке)
  byte contrastLVL_old = contrastLVL;
  lcd.clear();
  lcd.print("CONTRAST: ");
  lcd.print(contrastLVL);
  lcd.setCursor(0, 1);
  lcd.print("0123456789ABCDEF");
  while ((millis() - contrast_mills) < 10000) {
    if (contrastLVL_old != contrastLVL) {
      contrastLVL_old = contrastLVL;
      lcd.setCursor(10, 0);
      lcd.print("      ");
      lcd.setCursor(10, 0);
      lcd.print(contrastLVL);
    }
    delay(500);
  }
  EEPROM.update(contrastLVL_address, contrastLVL);
}

void btnPress_contrast() {
  contrast_mills = millis();
  contrastLVL += contrastLVL_step;
  if (contrastLVL >= contrastLVL_max) contrastLVL = contrastLVL_min;
  analogWrite(contrastPIN, contrastLVL);
}

void PrintToLCD(String str) {
  lcd.clear();
  lcd.print(str.substring(0,16));
  lcd.setCursor(0, 1);
  lcd.print(str.substring(16));
}

void setup() {

  contrastLVL = EEPROM.read(contrastLVL_address);
  if ((contrastLVL < contrastLVL_min) || (contrastLVL > contrastLVL_max)) contrastLVL = contrastLVL_default;
  pinMode(contrastPIN, OUTPUT);
  analogWrite(contrastPIN, contrastLVL);

  pinMode(interruptPin, INPUT_PULLUP);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Print a message to the LCD.
  lcd.print("Initializing... ");
  lcd.setCursor(0, 1);
  lcd.print(VERSION);

   delay(2000);

  if (!digitalRead(interruptPin)) {
    set_contrast();
  }

  Serial.setTimeout(COMREADTIMEOUT);
  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(interruptPin), btnPress_normal, FALLING);

  lcd.clear();
  lcd.print("     Ready!     ");

  lcd.clear();
  PrintToLCD("50 Ts DL:100MB/sT:480Gb  S:580GB");
}

void loop() {
  input = Serial.readString();
  if (input.length() == 32 ) PrintToLCD(input);
}


void btnPress_normal() {

}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
