//#define BLYNK_DEBUG // Optional, this enables lots of prints
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
//#define BLYNK_TIMEOUT_MS    750
#define BLYNK_HEARTBEAT     30
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WidgetRTC.h>

// Hardware Serial on Mega, Leonardo, Micro...
//#define EspSerial Serial1

// or Software Serial on Uno, Nano...
//#include <SoftwareSerial.h>
//SoftwareSerial EspSerial(2, 3); // RX, TX
// Your ESP8266 baud rate:
//#define ESP8266_BAUD 115200
//ESP8266 wifi(&EspSerial);

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
// char auth[] = "85fe301f0dcb4539adc1404c25bf2df2"; // Mega
char auth[] = "6defb2a6aa4e48c7ae89a51dc696c682"; // WemosD1
//char server[] = "139.59.206.133";

// WiFi credentials.
char ssid[] = "SDWifi24G";
char pass[] = "29HaD8828";

//BlynkTimer blynkTimer;
WidgetRTC widgetRTC;
