#define SERIAL_DEBUG_ENABLED
#include "LogUtils.h"
#include "RTCmodule.h"
#include <RTClib.h>
#include <Time.h>
#include <TimeLib.h>

RTC_DS3231 rtc;

long int rtcSyncProvider() {
  DateTime now = rtc.now();
  return now.unixtime();
}

void setupRTC() {
  debugLogln(F("Setup DS3231RTC"));
  rtc.begin();

  if(timeStatus() != timeSet) {
    msgLogln(F("Time sync with the Blynk was unsuccessful, trying with RTC"));
    setSyncProvider(rtcSyncProvider); // the function to get the unix time from the RTC
    if (timeStatus() != timeSet) {
      msgLogln(F("Unable to sync with the RTC"));
    }
    else {
      msgLogln(F("RTC has set the system time"));
    }
  }
  else {
    msgLogln(F("RTC time synced with the Blynk time"));
//    DateTime now = DateTime(year(), month(), day(), hour(), minute(), second());
    rtc.adjust(now());
//    RTC.set(now());
  }
  //enable alarm interrupts on match; also sets status flags A1F,A2F
//  pinMode(RTC_ALARM_PIN, INPUT_PULLUP);
//  RTC.alarmInterrupt(1, 1); //enable alarm 1 interrupt A1IE
//  RTC.alarmInterrupt(2, 1); //enable alarm 2 interrupt A2IE

  //enable or disable the square wave output,
  // no square wave (NONE) sets ITCN bit=1
  //enables alarm interrupt on square wave pin set from A1F, A2F
  //digitalRead with INPUT_PULLUP; LOW is alarm interrupt

//  RTC.squareWave(SQWAVE_NONE);

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
//  RTC.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0, 0);
//  RTC.setAlarm(ALM1_MATCH_SECONDS, 30, 0, 0, 0);
}

unsigned char getAlarmStatus() {
  unsigned char firedAlarm = 0;
//  if (digitalRead(RTC_ALARM_PIN) == 0) {
//    firedAlarm = RTC.alarm(1);
//    firedAlarm += 2 * RTC.alarm(2);
//    String logMessage = String(F("rtcAlarmPin=")) + RTC_ALARM_PIN; //resets with status reset
//    logMessage += String(F(" firedAlarm=")) + firedAlarm;                  //reads and resets status
//    DebugPrintln(logMessage);
//  }
  return firedAlarm;
}

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    //return pctime;
    if (pctime < DEFAULT_TIME) {
      // check the value is a valid time (greater than Jan 1 2013)
      pctime = 0L; // return 0 to indicate that the time is not valid
    }
    
    if (pctime != 0) {
      // set the RTC and the system time to the received value
      setTime(pctime);
//      DateTime now = DateTime(year(), month(), day(), hour(), minute(), second());
      rtc.adjust(pctime);
//      RTC.set(pctime); 
      
//      #if SERIAL_DEBUG_ENABLED == DEBUG_LOG_LVL
//        logMessage = F("RTC Synchronized to PC time=");
//        logMessage += pctime;
//        DebugPrintln(logMessage);
//      #endif
        msgLog(F("RTC Synchronized to PC time="));
        msgLogln(pctime);
    }
  }
  return pctime;
}
