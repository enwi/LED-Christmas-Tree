#pragma once

#include <ESPAsyncWebServer.h>

#if defined(ESP32)
#include <Update.h>
#else
#include <ESP8266WiFi.h>
#include <Updater.h>
#endif

class Networking
{
public:
    ///@brief Static class has no constructor
    Networking() = delete;

    ///@brief Create an access point with the given SSID
    ///
    ///@param ssid SSID of access point
    static void createAP(const char* ssid);

    ///@brief Handle the upload of binary program
    ///
    ///@param request Request coming from webserver
    ///@param filename Name of the uploading/uploaded file
    ///@param index Index of the raw @ref data within the whole 'file'
    ///@param data Raw data chunk
    ///@param len Size of the raw @ref data chunk
    static void handleOTAUpload(
        AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final);

private:
    static const IPAddress AP_IP;
    static const IPAddress NETMASK;
}; // namespace Networking
