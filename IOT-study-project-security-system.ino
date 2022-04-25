#include <LCD_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <PCF8574.h>
#include "Menu.h"
//#include "i2c-keypad-library-arduino/src/I2cKeypad.h"
#include "i2c-keypad-library-arduino/src/I2cKeypad.cpp"


/* ### DEFINE ### */

#define RX 8
#define TX A0
#define lcdColumns 16
#define lcdRows 2
#define KEYPAD_ADDRESS 0x23

#define SS_PIN D4
#define RST_PIN D0

#define KEYPAD_DEBOUNCE_TIME 20                // 20ms debounce time for the keypad
#define KEYPAD_ROWS 4
#define KEYPAD_COlS 4
byte colPins[KEYPAD_ROWS] = {0, 1, 2, 3};      // these are the i/o pins for each keypad row connected to the MCP23008 chip
byte rowPins[KEYPAD_COlS] = {4, 5, 6, 7};

/* ### GLOBAL OBJECTS ### */

int securityMode=0;
bool keyPressed = false;
uint8_t intPin = D8;


MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the rfid class
PCF8574 sensorMultiplexer(0x00); //Instance of the pcf8574 multiplexor class for sensors

char keyMap[KEYPAD_ROWS][KEYPAD_COlS] = {
  {'1','2','3','A'},                                // row 1 keys
  {'4','5','6','B'},                                // row 2 keys
  {'7','8','9','C'},                                // row 3 keys
  {'0','-','.','E'}                                 // row 4 keys
};

I2cKeypad keypad( ((char*)keyMap), rowPins, colPins,  KEYPAD_ROWS, KEYPAD_COlS, KEYPAD_DEBOUNCE_TIME, KEYPAD_ADDRESS);


LCD_I2C lcd(0x3f, lcdColumns, lcdRows);//standart addresses are 0x3f or 0x27


/* ### SENSORS ROUTINS ### */

void check_movement_sensor()
{
  if(sensorMultiplexer.digitalRead(P1))
    {lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Movement detected");
    delay(2000);}
    else{lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Movement not detected");
    delay(2000);}
}

void check_Hall_sensor()
{
  if(sensorMultiplexer.digitalRead(P2))
  {lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("The door is closed");
    delay(2000);}
    else
    {lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("The door is open");
    delay(2000);}
}


/* ### MENU ENTRIES ### */
struct menu_entry main_menu[4]={{"Press 1 for WiFi",&WiFi_control,0},{"Press 2 for RFID control", &RFID_control,1},{"Press 3 for sensors check", NULL,2},{"Press 4 for security mode change",NULL,3}};
struct menu_entry rfid_menu[2]={{"Press 1 to add new RFID",NULL,0},{"Press 2 to delete existing RFID", NULL,1}};
struct menu_entry sensors_menu[3]={{"Press 1 to check movement sensor",&check_movement_sensor,0},{"Press 2 to check Hall sensor", &check_Hall_sensor,1},{"Press 3 to check RFID",NULL,2}};


// Function interrupt
// void ICACHE_RAM_ATTR  keyPressedOnPCF8574();

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

void ICACHE_RAM_ATTR keyPressedOnPCF8574()
{
   keyPressed = true;
}

//void initialize_keypad()
//{
//  lcd.setCursor(0,1);
//  lcd.print("Keypad initialization");
//  //pinMode(D8, INPUT);
//  delay(500);
//  pinMode(intPin, INPUT);
//  attachInterrupt(digitalPinToInterrupt(intPin), keyPressedOnPCF8574, FALLING);
//  lcd.setCursor(0,1);
//  lcd.print("Interrupt attached");
//  delay(1000);
//  keypad.begin(); 
//}

int read()
{/*
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Key wait");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Key pressed");
  keyPressed= false;
  int c=keyPad.getKey();
  lcd.setCursor(0,0);
  lcd.print(c);
  delay(2000);
  return (c-52);*/
  
  return -1;
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
/*if(key!=-1 && key<size)
{
  lcd.clear();
(*main_menu[key].action)();
key=-1;
}
*/
//running_line++;
//running_line%=(size-1);
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
  //delay(10000);
  print_LCD("starting...",0,0);
  delay(1000);
  keypad.begin();                       // initialize the keypad
  delay(1000);
  print_LCD("keypad ok...",0,1);
  delay(1000);
  //initialize_keypad();
  sensorMultiplexer.pinMode(B01111111, INPUT);
  sensorMultiplexer.begin();
}


void loop() 
{
  String detected = "NO PRESS (0)";
  uint8_t keyFromKeypad1 = -1; // this is the returned ASCII value entered for the key pressed on the keypad. If no key pressed, binary 0 is returned.
  keyFromKeypad1 = keypad.getKey();
  if (keyFromKeypad1 != RETURN_NO_KEY_IN_BUFFER)               // if a key is pressed, display it (else we just continue on)
  {
    detected = "PRESS (1)";
  } 
  delay(100);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(detected);
  
//  print_menu(main_menu,4);
//  keypad.scanKeys();  
//  
//   uint8_t keyFromKeypad; // this is the returned ASCII value entered for the key pressed on the keypad. If no key pressed, binary 0 is returned.
//  keypad.flushKeys(); // Removes any keys from the keypad buffer. May not be necessary if you don't care if keys were already in the buffer.
//  
//  // call the getKeyUntil() function. This function also calls scanKeys() many times, so you don't need to call scanKeys().
//  // This function waits for one of the termination conditions to occur, and then returns (i.e. this function blocks other code from running until it is done).
//  // Note: If running a Particle device, this function getKeyUntil() will automatically call Particle.process() to keep the cloud connection alive.
//  lcd.clear();
//  lcd.setCursor(0,0);
//  lcd.print("wait");
//  keyFromKeypad = keypad.getKeyUntil(2000);
//  lcd.clear();
//  lcd.setCursor(0,0);
//  lcd.print("Pizdes1");
//  if (keyFromKeypad != RETURN_NO_KEY_IN_BUFFER)  // if a key is pressed, display it.
//  {
//    // display the returned ASCII keypad character.
//    lcd.clear();
//    lcd.setCursor(0,0);
//    lcd.print("The returned character is: ");
//    lcd.setCursor(0,1);
//    lcd.println(char(keyFromKeypad));
//    delay(5000);
//  }
//  else{lcd.clear();
//  lcd.setCursor(0,0);
//  lcd.print("Pizdes2");}
//  
//  lcd.clear();
//  lcd.setCursor(0,0);
//  lcd.print("Pizdes3");
}
