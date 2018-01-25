#ifndef LogUtils_h
  #define LogUtils_h

  #define ERROR_LOG_LVL 0
  #define WARNING_LOG_LVL 1
  #define DEBUG_LOG_LVL 2

  #ifdef SERIAL_DEBUG_ENABLED
    #define ErrorPrint(...)  \
          Serial.print(F("ERROR: "));    \
          Serial.print(__PRETTY_FUNCTION__); \
          Serial.print('(');      \
          Serial.print(__LINE__);     \
          Serial.print(F("): "));      \
          Serial.print(__VA_ARGS__)
    #define ErrorPrintln(...)  \
          Serial.print(F("ERROR: "));    \
          Serial.print(__PRETTY_FUNCTION__); \
          Serial.print('(');      \
          Serial.print(__LINE__);     \
          Serial.print(F("): "));      \
          Serial.println(__VA_ARGS__)
  #else
    #define ErrorPrint(...)
    #define ErrorPrintln(...)  
  #endif

  #if defined(SERIAL_DEBUG_ENABLED) && SERIAL_DEBUG_ENABLED > 0
    #define WarningPrint(...)  \
          Serial.print(F("WARNING: "));    \
          Serial.print(__PRETTY_FUNCTION__); \
          Serial.print('(');      \
          Serial.print(__LINE__);     \
          Serial.print(F("): "));      \
          Serial.print(__VA_ARGS__)
    #define WarningPrintln(...)  \
          Serial.print(F("WARNING: "));    \
          Serial.print(__PRETTY_FUNCTION__); \
          Serial.print('(');      \
          Serial.print(__LINE__);     \
          Serial.print(F("): "));      \
          Serial.println(__VA_ARGS__)
  #else
    #define WarningPrint(...)
    #define WarningPrintln(...)  
  #endif

  #if defined(SERIAL_DEBUG_ENABLED) && SERIAL_DEBUG_ENABLED > 1
    #define DebugPrint(...)  \
          Serial.print(F("DEBUG: "));    \
          Serial.print(__PRETTY_FUNCTION__); \
          Serial.print('(');      \
          Serial.print(__LINE__);     \
          Serial.print(F("): "));      \
          Serial.print(__VA_ARGS__)
    #define DebugPrintln(...)  \
          Serial.print(F("DEBUG: "));    \
          Serial.print(__PRETTY_FUNCTION__); \
          Serial.print('(');      \
          Serial.print(__LINE__);     \
          Serial.print(F("): "));      \
          Serial.println(__VA_ARGS__)
  #else
    #define DebugPrint(...)
    #define DebugPrintln(...)  
  #endif
#endif