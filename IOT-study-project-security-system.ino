#include <LCD_I2C.h>
#include <Wire.h>
#include <Arduino.h>
#include <I2CKeyPad.h>
#include <ESP8266WiFi.h>
#include "Menu.h"

/* ### DEFINE ### */

#define RX 8
#define TX A0
#define lcdColumns 16
#define lcdRows 2
#define KEYPAD_ADDRESS 0x27

// For arduino uno only pin 1 and 2 are interrupted
#define ARDUINO_UNO_INTERRUPTED_PIN D3

/* ### GLOBAL OBJECTS ### */

int securityMode=0;
bool keyPressed = false;


/* ### MENU ENTRIES ### */
struct menu_entry main_menu[4]={{"Press 1 for WiFi",&WiFi_control,0},{"Press 2 for RFID control", &RFID_control,1},{"Press 3 for sensors check", NULL,2},{"Press 4 for security mode change",NULL,3}};
struct menu_entry rfid_menu[2]={{"Press 1 to add new RFID",NULL,0},{"Press 2 to delete existing RFID", NULL,1}};
struct menu_entry sensors_menu[3]={{"Press 1 to check IR sensor",NULL,0},{"Press 2 to check Hall sensor", NULL,1},{"Press 3 to check RFID",NULL,2}};


// Function interrupt
void ICACHE_RAM_ATTR  keyPressedOnPCF8574();

I2CKeyPad keyPad(KEYPAD_ADDRESS);
char keys[] = "123A456B789C*0#DNF";  // Keypad layout. N = NoKey, F = Fail (e.g. >1 keys pressed)

LCD_I2C lcd(0x3f, lcdColumns, lcdRows);//standart addresses are 0x3f or 0x27


/* ### PCF8574 MULTIPLEXERS ### */

void initialize_keypad()
{
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  attachInterrupt(1, keyPressedOnPCF8574, FALLING);
  keyPad.begin();
}

int read()
{
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("Key wait");
    delay(2000);
  if (keyPressed)
  {
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("Key pressed");
    keyPressed= false;
    char c=keyPad.getKey();
    lcd.setCursor(0,0);
    lcd.print(c);
    delay(20000);
    return (c-52);
  }
  return -1;
}

void keyPressedOnPCF8574()
{
    lcd.clear();
    lcd.backlight();
    lcd.backlight();
    lcd.backlight();
    lcd.backlight();
    lcd.backlight();
    
   keyPressed = true;
}

/* ### I2C SCANNER ### */

int scanner() 
{
  byte error, address;
  int nDevices;
  int screenPosition=0;
  nDevices = 0;
  lcd.clear();
  for(address = 1; address < 127; address++ ) 
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
      if (error == 0) 
      {
        nDevices++;
        lcd.setCursor(screenPosition, 1);
        screenPosition+=2;
        lcd.print(address);
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

inline void print_LCD(const String str, int n, const int &row, const int &col)
{
    lcd.setCursor(0, 0);
    lcd.print(str);
    lcd.print(n);
}

void print_LCD(const String str, const int &row, const int &col)
{
    lcd.setCursor(row, col);
    lcd.print(str);
}

void print_menu(struct menu_entry* menu, int size)
{
  int running_line=0;
  int key=-1;
  while(true){
print_LCD(menu[running_line].text, 0, 0);
print_LCD(menu[running_line+1].text, 0, 1);
key=read();
delay(1000);
if(key!=-1 && key<size)
{
  lcd.clear();
(*main_menu[key].action)();
key=-1;
}

running_line++;
running_line%=(size-1);
lcd.clear();}
}


/* ### WIFI ### */

int scanWifiNetworks(){
  WiFi.scanDelete();
  return WiFi.scanNetworks(); //return number of networks found
}

bool isScanWifiNetworksDone(){
  return WiFi.scanComplete();
}

String createWifiNetworkInfoString(int i){
  return WiFi.SSID(i) + String(WiFi.RSSI(i)) + "db";
}

wl_status_t connectToWifiNetwork(int i, String password){
  return WiFi.begin(WiFi.SSID(i), password);
}

wl_status_t getWifiNetworkConnectionStatus(){
  return WiFi.status();
}


/* ### SECURITY MODE SET ### */

void security_mode_set()
{
  lcd.clear();
  if(securityMode==0)
    {securityMode=1;
  print_LCD("The security mode is ON", 0, 0);}
  else {securityMode=0;
  print_LCD("The security mode is OFF", 0, 0);}
  delay(30000);
}


/* ### MAIN SUPERLOOP ### */

void setup() 
{
  delay(2000);
  Wire.begin();

  lcd.begin();
  lcd.backlight();

  print_LCD("starting...",0,0);
  delay(1000);
  initialize_keypad();
}


void loop() 
{
  print_menu(main_menu,4); 
}
