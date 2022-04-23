#include <LCD_I2C.h>
#include <Wire.h>
#include "Arduino.h"
#include "PCF8574.h"

// For arduino uno only pin 1 and 2 are interrupted
#define ARDUINO_UNO_INTERRUPTED_PIN D3

// Function interrupt
void ICACHE_RAM_ATTR  keyPressedOnPCF8574();

// Set i2c address
PCF8574 pcf8574(0x38, ARDUINO_UNO_INTERRUPTED_PIN, keyPressedOnPCF8574);

 
#define RX 8
#define TX A0
#define lcdColumns 16
#define lcdRows 2


LCD_I2C lcd(0x3f, lcdColumns, lcdRows);//standart addresses are 0x3f or 0x27


int scanner() 
{
  byte error, address;
  int nDevices;
  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
      if (error == 0) 
      {
        nDevices++;
      }
        
  }
  return nDevices;   
}


byte find_nth_device(int n) 
{
  byte error, address;
  int nDevices;
  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error==0 && nDevices==n-1)
    {
      return address;  
    }
    if (error == 0) 
    {
      nDevices++;
    }
       
  }
  return 0;   
}


inline void print_LCD(const char* str, int n)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(str);
    lcd.print(n);
}

inline void print_LCD(const char* str)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(str);
}

inline int ask_for_input(const char *str)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(str);
    //TODO buttons press
}


void setup() 
{
  delay(2000);
  Wire.begin();

  lcd.begin();
  lcd.backlight();

  lcd.setCursor(0, 0);
  print_LCD("starting...");
  lcd.clear();
  initialize_PCF8574();
}

unsigned long timeElapsed;
void initialize_PCF8574()
{

  pcf8574.pinMode(P0, INPUT);
  pcf8574.pinMode(P1, INPUT_PULLUP);
  pcf8574.pinMode(P2, INPUT);
  pcf8574.pinMode(P3, INPUT);
  timeElapsed = millis();
}


bool keyPressed = false;
void read()
{
  if (keyPressed)
  {
    uint8_t val0 = pcf8574.digitalRead(P0);
    uint8_t val1 = pcf8574.digitalRead(P1);
    uint8_t val2 = pcf8574.digitalRead(P2);
    uint8_t val3 = pcf8574.digitalRead(P3);
    keyPressed= false;
  }
}



void keyPressedOnPCF8574(){
  // Interrupt called (No Serial no read no wire in this function, and DEBUG disabled on PCF library)
  print_LCD("Interrupt");
  delay(1000);
   keyPressed = true;

}
void loop() 
{ 
  read();
}
