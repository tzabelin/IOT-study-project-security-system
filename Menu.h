
extern LCD_I2C lcd;

/* ### FUNCTION TO PRINT MENU ###*/
extern void print_menu(struct menu_entry* menu, int size);
extern int scanWifiNetworks();
extern bool isScanWifiNetworksDone();
extern String createWifiNetworkInfoString(int i);
extern wl_status_t connectToWifiNetwork(int i, String password);
extern int menu_option;
extern void print_LCD(const String str, const int &row, const int &col);
extern String inputProcessor();
/* ### MENU ENTRY STRUCT ### */

struct menu_entry
{
String text;
void (*action)();
int entryNumber;
};


/* ### MENU SUBENTRIES FUNCTIONS ### */

void RFID_control()
{
  
}

void WiFi_connect()
{
  String password=inputProcessor();
    lcd.clear();
    print_LCD("Connection",0,0);
    delay(1000);
  if(connectToWifiNetwork(menu_option, password)==WL_CONNECTED)
  {lcd.clear();
    print_LCD("connection successful", 0, 0);
  delay(2000);
  return;}
  else{lcd.clear();
    print_LCD("connection failed", 0, 0);
  delay(2000);
    return;}
  
}

void WiFi_control()
{ 
  int numberOfNetworks=scanWifiNetworks();
  struct menu_entry* networks=new menu_entry[numberOfNetworks];
  for(int i=0;i<numberOfNetworks;i++)
  {
    networks[i].text=createWifiNetworkInfoString(i);
    networks[i].entryNumber=i;
    networks[i].action=&WiFi_connect;
  }
  WiFi_connect();
}
