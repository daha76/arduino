#ifndef RTCMODULE_H
	#define RTCMODULE_H
  #if defined(ARDUINO) && ARDUINO >= 100
    #include <Arduino.h>
  #else
    #include <WProgram.h>
  #endif	
  
  #define I2C_ADDR_RTC 0x68
  #define I2C_ADDR_EEPROM 0x57
  #define RTC_ALARM_PIN 10
  #define TIME_HEADER 'T' // Header tag for serial time sync message

  void setupRTC();
  unsigned char getAlarmStatus();
  unsigned long processSyncMessage();

#endif