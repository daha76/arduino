#define BLYNK_PRINT Serial

#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <WidgetRTC.h>

// Hardware Serial on Mega, Leonardo, Micro...
#define EspSerial Serial1

// or Software Serial on Uno, Nano...
//#include <SoftwareSerial.h>
//SoftwareSerial EspSerial(2, 3); // RX, TX

// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200

ESP8266 wifi(&EspSerial);

char auth[] = "XXXXXXXXXXXXXXXXXXXX"; // WemosD1
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "XXXXXXXX";
char pass[] = "XXXXXXXX";

BlynkTimer blynkTimer;
WidgetRTC widgetRTC;
