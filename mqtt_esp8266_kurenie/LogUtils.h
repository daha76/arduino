#ifndef LogUtils_h
  #define LogUtils_h

  #define msgLog(...) Serial.print(__VA_ARGS__)
  #define msgLogln(...) Serial.println(__VA_ARGS__)

  #ifdef SERIAL_DEBUG_ENABLED
    
    #define debugLogHeader()  \
          Serial.print(__PRETTY_FUNCTION__); \
          Serial.print('[');      \
          Serial.print(__LINE__);     \
          Serial.print("]: ")
    #define debugLog(...)  \
          Serial.print(__VA_ARGS__)
    #define debugLogln(...)  \
          Serial.println(__VA_ARGS__)
  #else
    #define debugLogHeader(...)
    #define debugLog(...)
    #define debugLogln(...)
  #endif

#endif
