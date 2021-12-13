#pragma once

#if defined(__AVR__)
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif

#define DEBUG_PRINT

#ifdef DEBUG_PRINT
#define DEBUG(s)                                                                                                       \
    {                                                                                                                  \
        Serial.print(s);                                                                                               \
    }
#define DEBUGLN(s)                                                                                                     \
    {                                                                                                                  \
        Serial.println(s);                                                                                             \
    }
#else
#define DEBUG(s)
#define DEBUGLN(s)
#endif

/// MAC adress of ESP8266
// size: 13 chars
extern char deviceMAC[13];

/// LED Christmas Tree
/// size: 18 chars
constexpr static const char* HOSTNAME PROGMEM = "LED Christmas Tree";