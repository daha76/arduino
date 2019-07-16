#include <OneWire.h>
#include <DHT.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SDA_PIN D2
#define SCL_PIN D1
#define I2C_ADDR_LCD 0x27
LiquidCrystal_I2C lcd(I2C_ADDR_LCD, 20, 4);

#define RELAYPIN D6

#define DS18B20PIN D5
#define TEMPERATURE_PRECISION 9
OneWire oneWire(DS18B20PIN);
DallasTemperature sensors(&oneWire);

#define DHTPIN D4 //pin gpio 12 in sensor
#define DHTTYPE DHT22   // DHT 22 Change this if you have a DHT11
DHT dht(DHTPIN, DHTTYPE);

#define SECOND 1000
#define MINUTE 60*SECOND
#define LCDINFO_TASK_TIME 3*SECOND
#define MEASSURE_TASK_TIME 17*SECOND
#define MQTT_TASK_TIME 53*SECOND
#define MQTT_RECONNECT_TASK_TIME 11*SECOND
#define BLYNK_RECONNECT_TASK_TIME MINUTE

const char* mqtt_server = "192.168.1.12";
const char* outTopic = "espChalupka/out";
const char* inTopic = "espChalupka/in";
