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

/// OTA web page
constexpr static const char* OTA_INDEX PROGMEM
    = R"(<!DOCTYPE html><html><head><meta charset=utf-8><title>OTA</title></head><body><div class="upload"><form method="POST" action="/ota" enctype="multipart/form-data"><input type="file" accept=".bin, application/octet-stream" name="data"/><input type="submit" name="upload" value="Upload" title="Upload Files"></form></div></body></html>)";