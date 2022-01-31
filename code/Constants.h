#pragma once

#if defined(__AVR__)
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif

#define DEBUG_SERIAL Serial

#ifdef DEBUG_SERIAL
#define DEBUG_PRINT
#define DEBUG(s) DEBUG_SERIAL.print(s)
#define DEBUGLN(s) DEBUG_SERIAL.println(s)
#if defined(__cplusplus) && (__cplusplus > 201703L)
#define DEBUGF(format, ...) DEBUG_SERIAL.printf(format, __VA_OPT__(, ) __VA_ARGS__)
#else // !(defined(__cplusplus) && (__cplusplus >  201703L))
#define DEBUGF(format, ...) DEBUG_SERIAL.printf(format, ##__VA_ARGS__)
#endif
#else
#define DEBUG(s)
#define DEBUGLN(s)
#define DEBUGF(format, ...)
#endif


/// MAC adress of ESP8266
// size: 13 chars
extern char deviceMAC[13];

/// LED Christmas Tree
/// size: 18 chars
constexpr static const char* HOSTNAME PROGMEM = "LED Christmas Tree";