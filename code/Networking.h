#pragma once

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>

#include "Config.h"
#include "Constants.h"
#include "Mqtt.h"
#include "TreeLight.h"

#include "webui/cpp/build.html.gz.h"

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

    static void initWifi();
    static void initServer(AsyncWebServer* server, TreeLight* light);

    static void getStatusJsonString(JsonObject& output);

    ///@brief Handle the upload of binary program
    ///
    ///@param request Request coming from webserver
    ///@param filename Name of the uploading/uploaded file
    ///@param index Index of the raw @ref data within the whole 'file'
    ///@param data Raw data chunk
    ///@param len Size of the raw @ref data chunk
    static void handleOTAUpload(
        AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final);

    ///@brief Handle the index page
    ///@param request Request coming from webserver
    static void handleIndex(AsyncWebServerRequest* request);

    ///@brief Handle the status api
    ///
    ///@param request  Request coming from webserver
    ///@param light TreeLight to control
    static void handleStatusApi(AsyncWebServerRequest* request, TreeLight* light);

    ///@brief Handle the config GET api
    ///@param request Request coming from webserver
    static void handleConfigApiGet(AsyncWebServerRequest* request);

    ///@brief Handle the config POST api
    ///
    ///@param request Request coming from webserver
    ///@param json JSON object containing configuration of wifi, mqtt, etc.
    static void handleConfigApiPost(AsyncWebServerRequest* request, JsonVariant* json);

    ///@brief Handle the set leds api
    ///
    ///@param request Request coming from webserver
    ///@param json JSON object containing values of brightness, speed, effect, etc.
    ///@param light TreeLight to control
    static void handleSetLedsApi(AsyncWebServerRequest* request, JsonVariant* json, TreeLight* light);

    /// @brief Check if the given string is an ip address
    ///
    /// @param str String to check
    /// @return true If the string is an ip
    /// @return false If the string is not an ip
    static bool isIp(const String& str);

    ///@brief Update DNS and other networking stuff
    ///
    /// Should be called once every second
    static void update();

private:
    /// @brief Callback used for captive portal webserver
    ///
    /// @param request Request to check and handle captive portal for
    /// @return true If not handling captive portal
    /// @return false If handling captive portal
    static bool captivePortal(AsyncWebServerRequest* request);

private:
    static const IPAddress AP_IP;
    static const IPAddress AP_NETMASK;
    static DNSServer dnsServer; // DNS server for captive portal
}; // namespace Networking
