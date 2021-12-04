#if defined(ESP8266) || defined(ESP32)
#include "Networking.h"

const IPAddress Networking::AP_IP = {4, 3, 2, 1};
const IPAddress Networking::NETMASK = {255, 255, 255, 0};

void Networking::createAP(const char* ssid)
{
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_IP, NETMASK);
    WiFi.softAP(ssid);

    // uint8_t timeout = 5;
    // do
    // {
    //     delay(500);
    // } while (--timeout);
}

void Networking::handleOTAUpload(
    AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final)
{
    if (!index)
    {
        Serial.printf("UploadStart: %s\n", filename.c_str());
        // calculate sketch space required for the update, for ESP32 use the max constant
#if defined(ESP32)
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
#else
        const uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace))
#endif
        {
            // start with max available size
            Update.printError(Serial);
        }
#if defined(ESP8266)
        Update.runAsync(true);
#endif
    }

    if (len)
    {
        Update.write(data, len);
    }

    // if the final flag is set then this is the last frame of data
    if (final)
    {
        if (Update.end(true))
        {
            // true to set the size to the current progress
            Serial.printf("Update Success: %ub written\nRebooting...\n", index + len);
            ESP.restart();
        }
        else
        {
            Update.printError(Serial);
        }
    }
}

#endif
