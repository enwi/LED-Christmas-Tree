#pragma once

#include <ArduinoJson.h>
#ifdef ESP32
    #include <AsyncTCP.h>
    #include <WiFi.h>
    #include <Update.h>
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESPAsyncTCP.h>
    #include <Updater.h>
    #include <include/WiFiState.h>
#endif
#include "ESPAsyncWebServer.h"

#include "Config.h"
#include "Constants.h"
#include "Mqtt.h"
#include "TreeLight.h"

class Networking
{
public:
    ///@brief Static class has no constructor
    Networking(Config& config) : config(config), mqtt(config) { }

    Networking(Networking&&) = delete;

    void initWifi();
    void initServer(TreeLight& light);

    void stop();
    void resume();
    void initOrResume(TreeLight& light);

    void getStatusJsonString(JsonObject& output);

    ///@brief Handle the upload of binary program
    ///
    ///@param request Request coming from webserver
    ///@param filename Name of the uploading/uploaded file
    ///@param index Index of the raw @ref data within the whole 'file'
    ///@param data Raw data chunk
    ///@param len Size of the raw @ref data chunk
    void handleOTAUpload(
        AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final);

    ///@brief Handle the index page
    ///@param request Request coming from webserver
    void handleIndex(AsyncWebServerRequest* request);

    ///@brief Handle the status api
    ///
    ///@param request  Request coming from webserver
    ///@param light TreeLight to control
    void handleStatusApi(AsyncWebServerRequest* request, TreeLight* light);

    ///@brief Handle the config GET api
    ///@param request Request coming from webserver
    void handleConfigApiGet(AsyncWebServerRequest* request);

    ///@brief Handle the config POST api
    ///
    ///@param request Request coming from webserver
    ///@param json JSON object containing configuration of wifi, mqtt, etc.
    void handleConfigApiPost(AsyncWebServerRequest* request, JsonVariant& json);

    ///@brief Handle the set leds api
    ///
    ///@param request Request coming from webserver
    ///@param json JSON object containing values of brightness, speed, effect, etc.
    ///@param light TreeLight to control
    void handleSetLedsApi(AsyncWebServerRequest* request, JsonVariant& json, TreeLight& light);

    /// @brief Check if the given string is an ip address
    ///
    /// @param str String to check
    /// @return true If the string is an ip
    /// @return false If the string is not an ip
    bool isIp(const String& str);

    /// @brief Check if Wifi should be on on startup
    /// This is the case if it was on at the last shutdown
    bool shouldEnableWifiOnStartup();

    ///@brief Update DNS and other networking stuff
    ///
    /// Should be called once every second
    void update();

private:
    /// @brief Callback used for captive portal webserver
    ///
    /// @param request Request to check and handle captive portal for
    /// @return true If not handling captive portal
    /// @return false If handling captive portal
    bool captivePortal(AsyncWebServerRequest* request);

    /// @brief Launch access point if client connection fails
    /// @return true If connected as client
    /// @return false If connection failed and access point was opened
    bool handleClientFailsafe();

    /// @brief Configure wifi for client mode
    void startClient();
    /// @brief Configure wifi for access point mode
    void startAccessPoint(bool persistent = true);

private:
    const IPAddress AP_IP = {192, 168, 4, 1};
    const IPAddress AP_NETMASK = {255, 255, 255, 0};
    DNSServer dnsServer; // DNS server for captive portal
    AsyncWebServer server {80}; /// Webserver for OTA
    bool isInitialized = false;
    Config& config;
    Mqtt mqtt;
    bool restartESP = false; /// Restart ESP after config change
}; // namespace Networking
