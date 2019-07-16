#define SERIAL_DEBUG_ENABLED
#define _TASK_PRIORITY
#define _TASK_STD_FUNCTION
#include "LogUtils.h"
#include "Settings.h"
#include "BlynkSettings.h"
#include "RTCmodule.h"
#include <TaskScheduler.h>
#include <TaskSchedulerDeclarations.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);

Scheduler taskManager, hpTaskManager;
void loopTask();
void mqttReconnectTask();
void blynkReconnectTask();

void lcd_info_1();
void lcd_backlight();
void dallasTask();
void mqttReportHeapTask();

Task t_loopTask(0, TASK_FOREVER, &loopTask, &hpTaskManager, false);
Task t_mqttReconnect(MQTT_RECONNECT_TASK_TIME, TASK_FOREVER, &mqttReconnectTask, &hpTaskManager, false);
Task t_blynkReconnect(BLYNK_RECONNECT_TASK_TIME, TASK_FOREVER, &blynkReconnectTask, &hpTaskManager, false);

Task t_LCD(LCDINFO_TASK_TIME, TASK_FOREVER, &lcd_info_1, &taskManager, false);
Task t_LCD_backlight(LCDINFO_TASK_TIME*10, TASK_FOREVER, &lcd_backlight, &taskManager, false);
Task t_meassure(MEASSURE_TASK_TIME, TASK_FOREVER, &dallasTask, &taskManager, false);
Task t_mqttReportHeap(MQTT_TASK_TIME, TASK_FOREVER, &mqttReportHeapTask, &taskManager, false);

float *dallasTemps = 0;
int numOfSensors = 0;
float dht22Temp = 0;
float dht22Hum = 0;
unsigned long previousMillis = 0;
boolean hasMeassuredData = false;

DeviceAddress tempDeviceAddress;

BLYNK_CONNECTED() {
  // Synchronize time on connection
  widgetRTC.begin();
  setSyncInterval(60*60);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  debugLog("Message arrived [");
  debugLog(topic);
  debugLog("] ");
  for (int i = 0; i < length; i++) {
    debugLog((char)payload[i]);
  }
  debugLogln("");
}

void setup() {
  Serial.begin(9600); // See the connection status in Serial Monitor
  while (!Serial || millis() < 1000) {
    ; // wait for serial port to connect. Needed for native USB
  }
//  WiFi.begin(ssid, pass);
  Blynk.connectWiFi(ssid, pass);
  Blynk.config(auth);//, ssid, pass); //insert here your SSID and password
  setSyncInterval(10 * 60);

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
    
  Wire.begin (SDA_PIN, SCL_PIN);
  lcd.init(); // initialize the lcd
  lcd.setBacklight(HIGH);
  lcd.clear();
  lcd.home(); // go home
  lcd.setCursor(5, 1);
  lcd.print(F("Hello :-)"));

  sensors.begin();
  delay(200);
  numOfSensors = sensors.getDeviceCount();
  if (dallasTemps != 0) {
    delete[] dallasTemps;
  }  
  dallasTemps = new float[numOfSensors];
  msgLog(F("Pocet DS18B20 senzorov:"));
  msgLogln(numOfSensors);

  for (int i=0; i < numOfSensors; i++) {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i)) {
      debugLog("Found device ");
      debugLog(i + 1, DEC);
      debugLog(" with address: ");
      printAddress(tempDeviceAddress);
      debugLogln("");
    
      debugLog("Setting resolution to ");
      debugLogln(TEMPERATURE_PRECISION, DEC);
    
      // set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
      sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
    
      debugLog("Resolution actually set to: ");
      debugLogln(sensors.getResolution(tempDeviceAddress), DEC); 
      char temp;
      temp = sensors.getHighAlarmTemp(tempDeviceAddress);
      debugLog("High Alarm: ");
      debugLog(temp, DEC);
      debugLog("C");
      debugLog(" Low Alarm: ");
      temp = sensors.getLowAlarmTemp(tempDeviceAddress);
      debugLog(temp, DEC);
      debugLogln("C");
    } 
    else {
      debugLog("Found ghost device at ");
      debugLog(i, DEC);
      debugLogln(" but could not detect address. Check power and cabling");
    }
  }

  lcd.clear();
  lcd.home();
  lcd.print(F("Pocet senzorov:"));
  lcd.print(numOfSensors);

  setupRTC();

  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW);

  taskManager.setHighPriorityScheduler(&hpTaskManager);
  t_loopTask.enable();
  t_meassure.enable();
}

void loop() {
  if(hasMeassuredData) t_LCD.enableIfNot();
  if (millis() - previousMillis > 1000) {
    previousMillis = millis();
    TimeDateDisplay(now());
  }
  
  if (Serial.available()) {
    processSyncMessage();
  }

//  blynkTimer.run();
  taskManager.execute();  
}

void loopTask() {
  if (Blynk.connected()) {
    Blynk.run();
  }
  else {
    lcd.setBacklight(HIGH);
    t_blynkReconnect.enableIfNot();
  }
 
  if (mqttClient.connected()) {
    mqttClient.loop();
    t_mqttReportHeap.enableIfNot();
  }
  else {
    lcd.setBacklight(HIGH);
    t_mqttReportHeap.disable();
    t_mqttReconnect.enableIfNot();
  }
}

void dallasTask() {
  t_meassure.setCallback(&dht22Task);
  sensors.requestTemperatures();
  for (int i = 0; i < numOfSensors; i++) {
    dallasTemps[i] = sensors.getTempCByIndex(i);
    
    debugLog(F("DS18B20("));
    debugLog(i);
    debugLog(F(")="));
    debugLogln(dallasTemps[i]);
    Blynk.virtualWrite(5 + i, dallasTemps[i]);
  }
}

void dht22Task() {
  t_meassure.setCallback(&dallasTask);
  dht.begin();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    msgLogln(F("Failed to read from DHT sensor!"));
    return;
  }
  dht22Temp = t;
  dht22Hum = h;
  float dp = getDewPointSlow(dht22Temp, dht22Hum);
  debugLog(F("DHT22:T="));
  debugLog(dht22Temp);
  debugLog(F(" H="));
  debugLog(dht22Hum);
  debugLog(F("% DP="));
  debugLogln(dp);
  
  Blynk.virtualWrite(0, dht22Temp);
  Blynk.virtualWrite(1, dht22Hum);
  Blynk.virtualWrite(2, dp);
  hasMeassuredData = true;
}

void mqttReconnectTask() {
  if(!mqttClient.connected()) {
    t_mqttReportHeap.disable();
    mqttClient.disconnect();
    msgLog(F("Attempting MQTT connection..."));
    if (mqttClient.connect("ESP8266Chalupka")) {
      msgLogln(F("Connected!"));

      mqttClient.publish(outTopic, "ESP8266Chalupka booted/reconnected");
      mqttClient.subscribe(inTopic);
      
      t_mqttReconnect.disable();
//      t_mqttReportHeap.enable();
      if(!t_LCD_backlight.isEnabled()) t_LCD_backlight.enableDelayed();
    } 
    else {
      msgLog(F("Failed! Status error="));
      msgLogln(mqttClient.state());
    }
  }  
}

void mqttReportHeapTask() {
  char msg[14];
  sprintf(msg, "freeHeap:%05d", ESP.getFreeHeap());
  mqttClient.publish(outTopic, msg);
  msgLogln(msg);  
}

void blynkReconnectTask() {
  if (!Blynk.connected()) {
    msgLog(F("[_\\|/_] Attempting BLYNK connection..."));
    if(Blynk.connect()) {
      msgLogln(F("Connected!"));
      t_blynkReconnect.disable();  
      if(!t_LCD_backlight.isEnabled()) t_LCD_backlight.enableDelayed();
    }
    else {
      msgLogln(F("Failed!"));
    }
  }  
}

void lcd_backlight() {
  debugLogln("Switch off LCD backlight.");
  lcd.setBacklight(LOW);
  t_LCD_backlight.disable();
}

void lcd_info_1() {
  for (int i = 0; i < numOfSensors; i++) {
    printTemp(i + 1, dallasTemps[i]);
  }
  for (int i = 3; i > numOfSensors; i--) {
    cleanRow(i);
  }
  t_LCD.setCallback(&lcd_info_2);
}

void lcd_info_2() {
  printTemp(1, dht22Temp);
  printHumidity(2, dht22Hum);
  cleanRow(3);
  t_LCD.setCallback(&lcd_info_1);
}

void printTemp(int row, float temp) {
  lcd.setCursor(0, row);
  lcd.print(F("Teplota: "));
  lcd.print(temp, 2);
}

void printHumidity(int row, float hum) {
  lcd.setCursor(0, row);
  lcd.print(F("Vlhkost: "));
  lcd.print(hum, 2);
}

void cleanRow(int row) {
  lcd.setCursor(0, row);
  lcd.print(F("                    "));
}

void TimeDateDisplay(time_t now) {
  //char timeBuf[17];
  //char dateBuf[17];
  char dateTimeBuf[21];

  //sprintf(timeBuf, "%02d:%02d:%02d", hour(now), minute(now), second(now));
  //sprintf(dateBuf, "%02d.%02d.%04d", day(now), month(now), year(now));
  sprintf(dateTimeBuf, "%02d.%02d.%04d  %02d:%02d:%02d", day(now), month(now), year(now), hour(now), minute(now), second(now));
  debugLogln(dateTimeBuf);
  lcd.setCursor(0, 0);
  lcd.print(dateTimeBuf);
}

double getDewPointSlow(float _temp, float _hum) {
    double a0 = (double) 373.15 / (273.15 + (double) _temp);
    double SUM = (double) -7.90298 * (a0-1.0);
    SUM += 5.02808 * log10(a0);
    SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/a0)))-1) ;
    SUM += 8.1328e-3 * (pow(10,(-3.49149*(a0-1)))-1) ;
    SUM += log10(1013.246);
    double VP = pow(10, SUM-3) * (double) _hum;
    double T = log(VP/0.61078); // temp var
    return (241.88 * T) / (17.558-T);
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) debugLog("0");
    debugLog(deviceAddress[i], HEX);
  }
}
