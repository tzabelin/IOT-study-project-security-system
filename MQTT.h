#include <PubSubClient.h>

extern void print_LCD(const String str, const int &row, const int &col);
extern LCD_I2C lcd;

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *topic, byte *payload, unsigned int length) {
  for (int i = 0; i < length; i++) 
  {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print((char) payload[i]);
      delay(2000);
  }
}

void MQTT_broker_start()
{
  const char *mqtt_broker = "broker.emqx.io";
  const char *topic = "esp8266/test";
  const char *mqtt_username = "emqx";
  const char *mqtt_password = "public";
  const int mqtt_port = 1883;

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) 
  {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) 
      {
          print_LCD("Public emqx mqtt broker connected",0,0);
          delay(2000);
      } else 
      {
          print_LCD("failed with state ",0,0);
          lcd.setCursor(1,0);
          lcd.print(client.state());
          delay(2000);
      }
}
}
