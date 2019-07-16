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

// char auth[] = "85fe301f0dcb4539adc1404c25bf2df2"; // Mega
char auth[] = "6defb2a6aa4e48c7ae89a51dc696c682"; // WemosD1
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "SDWifi24G";
char pass[] = "29HaD8828";

BlynkTimer blynkTimer;
WidgetRTC widgetRTC;
