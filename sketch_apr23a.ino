#include <LCD_I2C.h>
#include <Wire.h>

 
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
}


void loop() 
{ 
  
}
