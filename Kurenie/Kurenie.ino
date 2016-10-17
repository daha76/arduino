#include <Arduino.h>

#include <PciManager.h>
#include <PciListenerImp2.h>
#include <PciListenerImp.h>
#include <PciListener.h>
#include <IPciChangeHandler.h>
#include <DS3232RTC.h>
#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <SoftTimer.h>
#include <Heartbeat.h>
//#include <TonePlayer.h>
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
#define BACKLIGHT_PIN 3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

LiquidCrystal_I2C lcd(I2C_ADDR_LCD, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin, BACKLIGHT_PIN, POSITIVE);

#define I2C_ADDR_RTC 0x68
#define I2C_ADDR_EEPROM 0x57
#define RTC_ALARM_PIN 10
#define BEEPER_PIN  8

#define LOOP_TASK_TIME 0
#define DALLAS_TASK_TIME 60000
#define DHT22_TASK_TIME 60000
#define LCDINFO_TASK_TIME 2000
#define NUM_OF_VIEWS = 2

void loopTask(Task* me);
void dallasTask(Task* me);
void dht22Task(Task* me);
boolean lcd_info_1(Task *me);
boolean lcd_info_2(Task *me);

Task t_loop(LOOP_TASK_TIME, loopTask);
Task t_dallas(DALLAS_TASK_TIME, dallasTask);
Task t_DHT22(DHT22_TASK_TIME, dht22Task);

DelayRun t_lcd_info_1(LCDINFO_TASK_TIME, lcd_info_1);
DelayRun t_lcd_info_2(LCDINFO_TASK_TIME, lcd_info_2, &t_lcd_info_1);

Heartbeat heartbeat(BEAT_PIN);
//TonePlayer tonePlayer(BEEPER_PIN, 200);

float* dallasTemps = 0;
int numOfSensors = 0;
float dht22Temp = 0;
float dht22Hum = 0;
unsigned long startMillis;
unsigned long previousMillis = 0;

void setup() {
	startMillis = millis();
  pinMode(BEAT_PIN, OUTPUT );
  pinMode(RTC_ALARM_PIN, INPUT_PULLUP);
  Serial.begin(9600);

  sensors.begin();
  dht.begin();

  lcd.begin(20, 4);              // initialize the lcd
  lcd.setBacklight(HIGH);

  lcd.home ();                   // go home
  lcd.setCursor(5, 1);
  lcd.print(F("Hello :-)"));

  delay(1000);

  numOfSensors = sensors.getDeviceCount();
  Serial.print(F("NumOfSensors="));
  Serial.println(numOfSensors);
  if (dallasTemps != 0) {
    delete [] dallasTemps;
  }
  dallasTemps = new float[numOfSensors];

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

void loopTask(Task* me) {
	if (startMillis - previousMillis > 1000) {
		previousMillis = startMillis;
		time_t now = RTC.get();
		TimeDateDisplay(now);
	}
	startMillis = millis();

	if (Serial.available()) {
		time_t t = processSyncMessage();
		if (t != 0) {
			RTC.set(t);   // set the RTC and the system time to the received value
			Serial.print(F("RTC Synchronized to PC time="));
			Serial.println(t);
		}
	}

	checkAlarmStatus();
}

void dallasTask(Task* me) {
  sensors.requestTemperatures();
  for (int i = 0; i < numOfSensors; i++) {
    dallasTemps[i] = sensors.getTempCByIndex(i);
    Serial.print(F("Teplota S("));
    Serial.print(i);
    Serial.print(F(")="));
    Serial.println(dallasTemps[i]);
  }
}

void dht22Task(Task* me) {
  dht22Hum = dht.readHumidity();
  dht22Temp = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(dht22Hum) || isnan(dht22Temp)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("DHT22 - Teplota="));
  Serial.println(dht22Temp);
  Serial.print(F("DHT22 - Vlhkost="));
  Serial.println(dht22Hum);
}

boolean lcd_info_1(Task *me) {
  for (int i = 0; i < numOfSensors; i++) {
    printTemp(i+1, dallasTemps[i]);
  }
  return true;
}

boolean lcd_info_2(Task *me) {
  printTemp(1, dht22Temp);
  printHumidity(2, dht22Hum);
  return true;
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

void TimeDateDisplay(time_t now) {
	//char timeBuf[17];
	//char dateBuf[17];
	char dateTimeBuf[21];

	//sprintf(timeBuf, "%02d:%02d:%02d", hour(now), minute(now), second(now));
	//sprintf(dateBuf, "%02d.%02d.%04d", day(now), month(now), year(now));
	sprintf(dateTimeBuf, "%02d.%02d.%04d  %02d:%02d:%02d", day(now), month(now), year(now), hour(now), minute(now), second(now));
	Serial.println(dateTimeBuf);
	lcd.setCursor(0, 0);
	lcd.print(dateTimeBuf);
}

void setupRTC(void) {
  Serial.println("Setup DS3231RTC");
  setSyncProvider(RTC.get);   // the function to get the unix time from the RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");
  Serial.println();

  //enable alarm interrupts on match; also sets status flags A1F,A2F

  RTC.alarmInterrupt(1, 1); //enable alarm 1 interrupt A1IE
  RTC.alarmInterrupt(2, 1); //enable alarm 2 interrupt A2IE

  //enable or disable the square wave output,
  // no square wave (NONE) sets ITCN bit=1
  //enables alarm interrupt on square wave pin set from A1F, A2F
  //digitalRead with INPUT_PULLUP; LOW is alarm interrupt

  RTC.squareWave(SQWAVE_NONE);


  /*Alarm_Types defined in RTC3232.h.............................................

    ALM1_EVERY_SECOND
    ALM1_MATCH_SECONDS
    ALM1_MATCH_MINUTES     //match minutes *and* seconds
    ALM1_MATCH_HOURS       //match hours *and* minutes, seconds
    ALM1_MATCH_DATE        //match date *and* hours, minutes, seconds
    ALM1_MATCH_DAY         //match day *and* hours, minutes, seconds
    ALM2_EVERY_MINUTE
    ALM2_MATCH_MINUTES     //match minutes
    ALM2_MATCH_HOURS       //match hours *and* minutes
    ALM2_MATCH_DATE       //match date *and* hours, minutes
    ALM2_MATCH_DAY        //match day *and* hours, minutes
  */
  //setAlarm Format (Alarm_Type, seconds, minutes, hours, date(or day))
  RTC.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0, 0);
  RTC.setAlarm(ALM1_MATCH_SECONDS, 30, 0, 0, 0);

}

void checkAlarmStatus() {
  if (digitalRead(RTC_ALARM_PIN) == 0) {
    alarmStatus();
  }
}

void alarmStatus() {
  Serial.print("rtcAlarmPin   ");
  Serial.println(digitalRead(RTC_ALARM_PIN));//resets with status reset
  Serial.print("Alarm1 status  ");
  Serial.println(RTC.alarm(1));//reads and resets status
  Serial.print("Alarm2 status  ");
  Serial.println(RTC.alarm(2));//reads and resets status
  Serial.println();
}

/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    //return pctime;
    if ( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
      pctime = 0L; // return 0 to indicate that the time is not valid
    }
    setTime(pctime);
  }
  return pctime;
}

