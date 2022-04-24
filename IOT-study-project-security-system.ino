#include <LCD_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
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

#define SS_PIN D4
#define RST_PIN D0

// For arduino uno only pin 1 and 2 are interrupted
#define ARDUINO_UNO_INTERRUPTED_PIN D3

/* ### GLOBAL OBJECTS ### */

int securityMode;
int key_pressed;


/* ### MENU ENTRIES ### */
struct menu_entry main_menu[4]={{"Press 1 for WiFi",&WiFi_control,0},{"Press 2 for RFID control", &RFID_control,1},{"Press 3 for sensors check", NULL,2},{"Press 4 for security mode change",NULL,3}};
struct menu_entry rfid_menu[2]={{"Press 1 to add new RFID",NULL,0},{"Press 2 to delete existing RFID", NULL,1}};
struct menu_entry sensors_menu[3]={{"Press 1 to check IR sensor",NULL,0},{"Press 2 to check Hall sensor", NULL,1},{"Press 3 to check RFID",NULL,2}};


// Function interrupt
void ICACHE_RAM_ATTR  keyPressedOnPCF8574();

// Set i2c address
PCF8574 pcf8574(0x38, ARDUINO_UNO_INTERRUPTED_PIN, keyPressedOnPCF8574);
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
LCD_I2C lcd(0x3f, lcdColumns, lcdRows);//standart addresses are 0x3f or 0x27

/* ### MFRC522 RFID ### */

void initialize_MFRC522(){
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
}

bool isRfidPresent(){
  if (!rfid.PICC_IsNewCardPresent())
    return false;
    
  if (!rfid.PICC_ReadCardSerial())
    return false;
    
  return true;
}

// Check is the PICC of Classic MIFARE type
bool isRfidMifareClassic(){
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    //Serial.println(F("Your tag is not of type MIFARE Classic."));
    return false;
  } else {
    return true;
  }
}



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
  while(key_pressed==0){
print_LCD(menu[running_line].text, 0, 0);
print_LCD(menu[running_line+1].text, 0, 1);
key_pressed=read();
delay(1000);
key_pressed=read();
if(key_pressed!=0)
{
  lcd.clear();
(*main_menu[key_pressed-1].action)();
key_pressed=0;
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

/* ### UTILITIES ### */

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize) {
 for (byte i = 0; i < bufferSize; i++) {
   Serial.print(buffer[i] < 0x10 ? " 0" : " ");
   Serial.print(buffer[i], HEX);
 }
}
/**
   Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte *buffer, byte bufferSize) {
 for (byte i = 0; i < bufferSize; i++) {
   Serial.print(buffer[i] < 0x10 ? " 0" : " ");
   Serial.print(buffer[i], DEC);
 }
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
  
  key_pressed=0;
  securityMode=0;
}


void loop() 
{
  print_menu(main_menu,4); 
}
