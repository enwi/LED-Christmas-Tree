#pragma once

#if defined(ESP8266) || defined(ESP32)

#include <ArduinoJson.h>
#include <FS.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>


#include "Constants.h"

struct NetworkConfig
{
    bool clientEnabled = false;
    String clientSsid = "YourWifi";
    String clientPassword = "inputyourown";
    bool dhcpEnabled = true;
    IPAddress clientMask;
    IPAddress clientGateway;
    IPAddress clientDns;
    IPAddress clientIp;
    bool apEnabled = true;
    String apSsid;
    String apPassword;

    void fromJson(const JsonObjectConst& object);
    void toJson(JsonObject& object) const;
};

struct MqttConfig
{
    bool enabled = false;
    String server;
    int port = 1883;
    String id = "LedChristmasTree";
    String user;
    String password;

    void fromJson(const JsonObjectConst& object);
    void toJson(JsonObject& object) const;
};

class Config
{
public:
    ///@brief Static class has no constructor
    Config() = delete;

    static void initConfig();
    static NetworkConfig& getNetworkConfig();
    static MqttConfig& getMqttConfig();
    static void setDefaultConfig();
    static void save();
    static void createJson(JsonDocument& output);

private:
    static NetworkConfig networkConfig;
    static MqttConfig mqttConfig;
}; // namespace Networking

#endif