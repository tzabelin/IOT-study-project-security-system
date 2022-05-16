#include <LCD_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h> 
#include <EEPROM.h>
#include <PCF8574.h>
#include <I2CKeyPad.h>
#include "Menu.h"
#include "HtmlSource.h"
 
 
/* ### DEFINE ### */
 
#define RX 8
#define TX A0
#define lcdColumns 16
#define lcdRows 2
#define KEYPAD_ADDRESS 0x23
 
#define SS_PIN D4
#define RST_PIN D0
 
/* ### KEYBOARD LAYOUT ### */
 
// s n
// 1 2 3 h
// 4 5 6 d
// 7 8 9 e
//  0  . 
 
// Where: 
//  s - ESC
//  n - END
//  h - HM
//  d - DEL
//  e - ENTER
 
char keys[] = ".8520741edhn963sNF"; // N = NoKey, F = Fail (e.g. >1 keys pressed)
 
/* ### GLOBAL OBJECTS ### */
 
// Memory Structure
// 0     + byte               | RfidEntries size
// 1     + RfidEntry <FB>     | RfidEntries[0]
// 2     | ...                |
// 3     |                    |
// ...                        | RfidEntries[0..3] data 
// 116   + RfidEntry <LB>     | RfidEntries[3]
// 117   + byte               | ActionEntries size
// 118   + ActionEntry        | ActionEntries[0]
// 119   |                    |
// 120   |                    |
 
struct RfidEntry {
  byte nuid[4];
  char _name[24];
  byte role;
};
 
struct ActionEntry {
  int eventTime;
  byte type;
  byte nuid[4];
};
 
 
int securityMode = 0;
bool alertTriggered = false;
int menu_option;

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the rfid class
byte nuidPICC[4];
byte rfidEntriesSize;
RfidEntry rfidEntries[4];
 
PCF8574 sensorMultiplexer(0x20); //Instance of the pcf8574 multiplexor class for sensors
 
I2CKeyPad keyPad(KEYPAD_ADDRESS);
uint32_t lastKeyPressed = 0;
uint8_t intPin = D8;
char key = 'N';
 
LCD_I2C lcd(0x3f, lcdColumns, lcdRows);//standart addresses are 0x3f or 0x27
 
ESP8266WebServer server(80);
 void getAlert();
/* ### PCF8574 MULTIPLEXERS ### */
 
void initializeKeypad()
{
  lcd.setCursor(0,1);
  lcd.print("KEYPAD_BEGIN");
  keyPad.begin();
  delay(100);
  lcd.print("KEYPAD_READY");
}
 
void check_movement_sensor()
{
  
  if(sensorMultiplexer.read8() & B00000011)
  {lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("The movement is detected");
  if(securityMode==1)
  {getAlert();}

    delay(2000);}
    else
    {lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("The movement is not detected");
   delay(2000);}
}
 
void check_Hall_sensor()
{
  
  if(sensorMultiplexer.read8() & B00000011)
  {lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("The door is open");
  if(securityMode==1)
  {getAlert();}
    delay(2000);}
    else
    {lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("The door is closed");
   delay(2000);}
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
  uint32_t start = millis();
uint32_t now = millis();
  if (now - start <= 1000)
  {
    now=millis();
  }
}
 
/* ### MENU ENTRIES ### */
struct menu_entry main_menu[4]={{"WiFi control",&WiFi_control,0},{"RFID control", &RFID_control,1},{"Sensors check", &sensors,2},{"Security mode change",&security_mode_set,3}};
struct menu_entry rfid_menu[2]={{"Add new RFID",NULL,0},{"Delete existing RFID", NULL,1}};
struct menu_entry sensors_menu[3]={{"Movement sensor",&check_movement_sensor,0},{"Hall sensor", &check_Hall_sensor,1},{"RFID",NULL,2}};
 
/* ### Sensors menu function ### */
 
void sensors()
{
  print_menu(sensors_menu,3);
}
 
 
 
/* ### Web Server ### */
 
void handleRoot() {
  server.send(200, "text/html", htmlSourceString);
}
 
void getAlert() 
{
  server.send(200, "text/plane", String(alertTriggered));
}
 
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
 
/* ### EEPROM ### */
 
void initializeEEPROM(){
  EEPROM.begin(256);
}
 
void writeByteToEEPROM(int addr, byte val){
  EEPROM.write(addr, val);
  EEPROM.commit();
}
 
byte readByteFromEEPROM(int addr){
  byte val;
  EEPROM.get(addr, val);
  return val;
}
 
void writeIntToEEPROM(int addr, int val){
  EEPROM.put(addr, val);
  EEPROM.commit();
}
 
int readIntFromEEPROM(int addr){
  int val;
  EEPROM.get(addr, val);
  return val;
}
 
void writeRfidBufferToEEPROM(){
  writeByteToEEPROM(0, rfidEntriesSize); //0
 
  // RFID_E[0]: 1-29
  // RFID_E[1]: 30-58
  // RFID_E[2]: 59-87
  // RFID_E[3]: 88-116
  
  for (int i = 0; i < rfidEntriesSize; i++){
    EEPROM.put(i*29+1, rfidEntries[i]);
  }
  
  EEPROM.commit();
}
 
void readRfidBufferFromEEPROM(){
  EEPROM.get(0, rfidEntriesSize);
  
  for (int i = 0; i < 4; i++){
    EEPROM.get(i*29+1, rfidEntries[i]);
  }
}
 
 
void read()
{
  key='N';
  uint32_t now = millis();
 
  if (now - lastKeyPressed >= 100)
  {
    lastKeyPressed = now;
 
    //start = micros();
    uint8_t index = keyPad.getKey();
    //stop = micros();
 
    key=(keys[index]);
    
  }
}


String inputProcessor()
{
  int letter_number=key;
  int letter_case=0;
  String input="";
  if(key=='e')
  {
  return input;
  }
  if(letter_number>=1 && letter_number<=26)
  {
    if(letter_case==1) //upper case
    {
    input+=(char)(letter_number+40);
    }
    else //lower case
    {
    input+=(char)(letter_number+60);
    }
  }
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
    lcd.setCursor(1,0);
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
  print_LCD(menu[running_line].text, 0, 0);
  print_LCD(menu[(running_line+1)%(size)].text, 0, 1);
  while(true)
  {
  read();
  if(key=='e')
  {
    lcd.clear();
    menu_option=running_line;
    (*main_menu[running_line].action)();
  lcd.setCursor(0,0);
  lcd.print(running_line);
  lcd.setCursor(3,0);
  lcd.print(menu[running_line].text);
  
  lcd.setCursor(0,1);
  lcd.print((running_line+1)%(size));
  lcd.setCursor(3,1);
  lcd.print(menu[(running_line+1)%(size)].text);
  }
  if(key=='s')
  {
    lcd.clear();
    return;
  }
  if(key=='8')
  {running_line++;
  running_line%=(size);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(running_line);
  lcd.setCursor(3,0);
  lcd.print(menu[running_line].text);
  
  lcd.setCursor(0,1);
  lcd.print((running_line+1)%(size));
  lcd.setCursor(3,1);
  lcd.print(menu[(running_line+1)%(size)].text);
  }
  
  if(key=='2')
  {running_line--;
  if(running_line<0)
  {running_line=(size-1);}
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(running_line);
  lcd.setCursor(3,0);
  lcd.print(menu[running_line].text);
  
  lcd.setCursor(0,1);
  lcd.print((running_line+1)%(size));
  lcd.setCursor(3,1);
  lcd.print(menu[(running_line+1)%(size)].text);
  }
  
  }
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
 
 
/* ### UTILITIES ### */
 
void initWire(){
  Wire.begin();
  Wire.setClock(400000);
}
 
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
  
  initWire();
 
  lcd.begin();
  lcd.backlight();
  
  print_LCD("Starting...",0,0);
  initializeKeypad();
  
  sensorMultiplexer.begin();
}
 
 
void loop() 
{
  // Check keypress each 50ms
  
  // If key pressed:
  // 1. If menu - move menu (if 2 - up, 8 - down)
  // 2. If textfield - enter values or on "Enter" proceed to next menu element
 
  // We should store last key in a global variable. If key is pressed - value is assigned. 
  // Next functions check if value is assigned or not.
  // 1. If assigned - key was pressed. Unassign it to "N" (No Key)
  // 2. If not assigned - key was not pressed.
 
  
  
   print_menu(main_menu,4);
  //scanner();
  //server.handleClient();
}
 
