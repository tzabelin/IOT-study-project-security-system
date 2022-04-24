#include <LCD_I2C.h>
#include <Wire.h>
#include "Arduino.h"
#include "PCF8574.h"
#include "ESP8266WiFi.h"
#include "Menu.h"

/* ### DEFINE ### */

#define RX 8
#define TX A0
#define lcdColumns 16
#define lcdRows 2

// For arduino uno only pin 1 and 2 are interrupted
#define ARDUINO_UNO_INTERRUPTED_PIN D3

/* ### GLOBAL OBJECTS ### */

// Function interrupt
void ICACHE_RAM_ATTR  keyPressedOnPCF8574();

// Set i2c address
PCF8574 pcf8574(0x38, ARDUINO_UNO_INTERRUPTED_PIN, keyPressedOnPCF8574);

LCD_I2C lcd(0x3f, lcdColumns, lcdRows);//standart addresses are 0x3f or 0x27

struct menu_entry main_menu[4]={{"Press 1 for WiFi",NULL},{"Press 2 for RFID control", NULL},{"Press 3 for sensors check", NULL},{"Press 4 for security mode",NULL}};
struct menu_entry rfid_menu[2]={{"Press 1 to add new RFID",NULL},{"Press 2 to delete existing RFID", NULL}};
struct menu_entry sensors_menu[3]={{"Press 1 to check IR sensor",NULL},{"Press 2 to check Hall sensor", NULL},{"Press 3 to check RFID",NULL}};

int key;

/* ### PCF8574 MULTIPLEXERS ### */

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
int read()
{
  if (keyPressed)
  {
    if(pcf8574.digitalRead(P0))
      return 1;
    if(pcf8574.digitalRead(P1))
      return 2;
    if(pcf8574.digitalRead(P2))
      return 3;
    if(pcf8574.digitalRead(P3))
      return 4;
    keyPressed= false;
  }
  return 0;
}

void keyPressedOnPCF8574(){
  // Interrupt called (No Serial no read no wire in this function, and DEBUG disabled on PCF library)
  print_LCD("Interrupt",0,0);
  delay(1000);
   keyPressed = true;

}

/* ### I2C SCANNER ### */

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

/* ### 1604 LCD ### */

inline void print_LCD(const char* str, int n)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(str);
    lcd.print(n);
}

inline void print_LCD(const char* str, const int &row, const int &col)
{
    lcd.setCursor(row, col);
    lcd.print(str);
}

void print_menu(struct menu_entry* menu, int size)
{
  int running_line=0;
  while(true){
print_LCD(menu[running_line].text, 0, 0);
print_LCD(menu[running_line+1].text, 0, 1);
key=read();
delay(1000);
key=read();
//(*main_menu[key].action)();
running_line++;
running_line%=size;
lcd.clear();}
}
/* ### MAIN SUPERLOOP ### */

void setup() 
{
  delay(2000);
  Wire.begin();

  lcd.begin();
  lcd.backlight();

  lcd.setCursor(0, 0);
  print_LCD("starting...",0,0);
  lcd.clear();
  initialize_PCF8574();
  
  key=0;
}


void loop() 
{
  print_menu(main_menu,4); 
}
