#pragma once

#if defined(__AVR__)
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif

/// MAC adress of ESP8266
// size: 13 chars
extern char deviceMAC[13];

/// Online
/// size: 6 chars
constexpr static const char* ONLINE = "Online";

/// Offline
/// size: 7 chars
constexpr static const char* OFFLINE = "Offline";

/// LED Christmas Tree
/// size: 18 chars
constexpr static const char* HOSTNAME PROGMEM = "LED Christmas Tree";