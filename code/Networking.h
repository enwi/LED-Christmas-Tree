#pragma once

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

#include "Constants.h"
#include "Config.h"
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
    static void initServer(AsyncWebServer *server, TreeLight *light);

    static void getStatusJsonString(JsonObject &output);

    ///@brief Handle the upload of binary program
    ///
    ///@param request Request coming from webserver
    ///@param filename Name of the uploading/uploaded file
    ///@param index Index of the raw @ref data within the whole 'file'
    ///@param data Raw data chunk
    ///@param len Size of the raw @ref data chunk
    static void handleOTAUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final);

    ///@brief Handle the index page
    ///@param request Request coming from webserver
    static void handleIndex(AsyncWebServerRequest* request);

    ///@brief Handle the status api
    ///@param request Request coming from webserver
    static void handleStatusApi(AsyncWebServerRequest* request, TreeLight *light);

    ///@brief Handle the config GET api
    ///@param request Request coming from webserver
    static void handleConfigApiGet(AsyncWebServerRequest *request);

    ///@brief Handle the config POST api
    ///@param request Request coming from webserver
    static void handleConfigApiPost(AsyncWebServerRequest *request, JsonVariant *json);

    ///@brief Handle the set leds api
    ///@param request Request coming from webserver
    static void handleSetLedsApi(AsyncWebServerRequest *request, JsonVariant *json, TreeLight *light);

private:
    static const IPAddress AP_IP;
    static const IPAddress AP_NETMASK;
}; // namespace Networking
