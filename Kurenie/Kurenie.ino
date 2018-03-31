#define SERIAL_DEBUG_ENABLED DEBUG_LOG_LVL
#include "LogUtils.h"
#include "BlynkSettings.h"
#include "RTCmodule.h"
// #include <PciManager.h>
// #include <PciListenerImp2.h>
// #include <PciListenerImp.h>
// #include <PciListener.h>
// #include <IPciChangeHandler.h>
// #include <DS3232RTC.h>
//#include <Time.h>
// #include <TimeLib.h>
#include <Wire.h>
//#include <LCD.h>
#include <LiquidCrystal_I2C.h>
// #include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <SoftTimer.h>
#include <Heartbeat.h>
// #include <TonePlayer.h>
#include <DelayRun.h>

#define BEAT_PIN 13

#define DHTPIN 32
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

#define ONE_WIRE_BUS 30
#define TEMPERATURE_PRECISION 9
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire); // Initialize Dallas temperature sensor

#define I2C_ADDR_LCD 0x27
// #define BACKLIGHT_PIN 3
// #define En_pin 2
// #define Rw_pin 1
// #define Rs_pin 0
// #define D4_pin 4
// #define D5_pin 5
// #define D6_pin 6
// #define D7_pin 7

LiquidCrystal_I2C lcd(I2C_ADDR_LCD, 20, 4); //En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin, BACKLIGHT_PIN, POSITIVE);

#define BEEPER_PIN 8

#define LOOP_TASK_TIME 0
#define DALLAS_TASK_TIME 60000
#define DHT22_TASK_TIME 60000
#define LCDINFO_TASK_TIME 2000
#define NUM_OF_VIEWS 2

void loopTask(Task *me);
void dallasTask(Task *me);
void dht22Task(Task *me);
boolean lcd_info_1(Task *me);
boolean lcd_info_2(Task *me);

Task t_loop(LOOP_TASK_TIME, loopTask);
Task t_dallas(DALLAS_TASK_TIME, dallasTask);
Task t_DHT22(DHT22_TASK_TIME, dht22Task);

DelayRun t_lcd_info_1(LCDINFO_TASK_TIME, lcd_info_1);
DelayRun t_lcd_info_2(LCDINFO_TASK_TIME, lcd_info_2, &t_lcd_info_1);

Heartbeat heartbeat(BEAT_PIN);
//TonePlayer tonePlayer(BEEPER_PIN, 200);

float *dallasTemps = 0;
int numOfSensors = 0;
float dht22Temp = 0;
float dht22Hum = 0;
unsigned long previousMillis = 0;

#ifdef SERIAL_DEBUG_ENABLED
  String logMessage;
#endif

char *string2char(String msg)
{
  if (msg.length() != 0)
  {
    char *p = const_cast<char *>(msg.c_str());
    return p;
  }
}

BLYNK_CONNECTED() 
{
  // Synchronize time on connection
  widgetRTC.begin();
}

void setup()
{
  #ifdef SERIAL_DEBUG_ENABLED
    logMessage.reserve(50);
  #endif

  Serial.begin(9600);
  while (!Serial || millis() < 1000) {
    ; // wait for serial port to connect. Needed for native USB
  }

  // Set ESP8266 baud rate
  EspSerial.begin(ESP8266_BAUD);
  delay(10);
  Blynk.begin(auth, wifi, ssid, pass);
  setSyncInterval(10 * 60);
  
  pinMode(BEAT_PIN, OUTPUT);

  sensors.begin();
  sensors.setResolution(TEMPERATURE_PRECISION);
  dht.begin();

  lcd.begin(20, 4); // initialize the lcd
  lcd.setBacklight(HIGH);

  lcd.home(); // go home
  lcd.setCursor(5, 1);
  lcd.print(F("Hello :-)"));

  numOfSensors = sensors.getDeviceCount();
  
  #if SERIAL_DEBUG_ENABLED == DEBUG_LOG_LVL
    logMessage = F("NumOfSensors=");
    logMessage += numOfSensors;
    DebugPrintln(logMessage);
  #endif

  if (dallasTemps != 0)
  {
    delete[] dallasTemps;
  }
  dallasTemps = new float[numOfSensors];

  delay(1000);
  lcd.clear();
  lcd.home();
  lcd.print(F("Pocet senzorov:"));
  lcd.print(numOfSensors);

  setupRTC();
  delay(1000);
  lcd.clear();
  //tonePlayer.play("c1g1c1");//g1j2j2c1g1c1g1j2j2o1n1l1j1h2l2_2j1h1g1e1c2c2");

  SoftTimer.add(&t_loop);
  SoftTimer.add(&t_dallas);
  SoftTimer.add(&t_DHT22);

  t_lcd_info_1.followedBy = &t_lcd_info_2;
  //t_lcd_info_2.followedBy = &t_lcd_info_3;
  t_lcd_info_1.startDelayed();
}

void loopTask(Task *me)
{
  if (millis() - previousMillis > 1000)
  {
    previousMillis = millis();
    // time_t now = RTC.get();
    TimeDateDisplay(now());
  }

  if (Serial.available())
  {
    processSyncMessage();
  }

  checkAlarmStatus();
  Blynk.run();
  blynkTimer.run();
}

void dallasTask(Task *me)
{
  sensors.requestTemperatures();
  for (int i = 0; i < numOfSensors; i++)
  {
    dallasTemps[i] = sensors.getTempCByIndex(i);
    
    #if SERIAL_DEBUG_ENABLED == DEBUG_LOG_LVL
      logMessage = F("DS18B20(");
      logMessage += i;
      logMessage += F(")=");
      logMessage += dallasTemps[i];
      DebugPrintln(logMessage);
    #endif
    Blynk.virtualWrite(12 + i, dallasTemps[i]);
  }
}

void dht22Task(Task *me)
{
  dht22Hum = dht.readHumidity();
  dht22Temp = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(dht22Hum) || isnan(dht22Temp))
  {
    ErrorPrintln(F("Failed to read from DHT sensor!"));
    return;
  }
  
  #if SERIAL_DEBUG_ENABLED == DEBUG_LOG_LVL
    logMessage = F("DHT22:T=");
    logMessage += dht22Temp;
    logMessage += F(" H=");
    logMessage += dht22Hum;
    logMessage += '%';
    DebugPrintln(logMessage);
  #endif
  
  Blynk.virtualWrite(10, dht22Temp);
  Blynk.virtualWrite(11, dht22Hum);
}

boolean lcd_info_1(Task *me)
{
  for (int i = 0; i < numOfSensors; i++)
  {
    printTemp(i + 1, dallasTemps[i]);
  }
  return true;
}

boolean lcd_info_2(Task *me)
{
  printTemp(1, dht22Temp);
  printHumidity(2, dht22Hum);
  return true;
}

void printTemp(int row, float temp)
{
  lcd.setCursor(0, row);
  lcd.print(F("Teplota: "));
  lcd.print(temp, 2);
}

void printHumidity(int row, float hum)
{
  lcd.setCursor(0, row);
  lcd.print(F("Vlhkost: "));
  lcd.print(hum, 2);
}

void TimeDateDisplay(time_t now)
{
  //char timeBuf[17];
  //char dateBuf[17];
  char dateTimeBuf[21];

  //sprintf(timeBuf, "%02d:%02d:%02d", hour(now), minute(now), second(now));
  //sprintf(dateBuf, "%02d.%02d.%04d", day(now), month(now), year(now));
  sprintf(dateTimeBuf, "%02d.%02d.%04d  %02d:%02d:%02d", day(now), month(now), year(now), hour(now), minute(now), second(now));
  DebugPrintln(dateTimeBuf);
  lcd.setCursor(0, 0);
  lcd.print(dateTimeBuf);
}

void checkAlarmStatus()
{
    getAlarmStatus();
}
