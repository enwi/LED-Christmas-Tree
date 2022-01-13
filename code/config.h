#pragma once

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
    bool wifiEnabled = false;

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
    
    void initConfig();
    NetworkConfig& getNetworkConfig();
    MqttConfig& getMqttConfig();
    void setDefaultConfig();
    void save();
    void createJson(JsonDocument& output);

private:
    NetworkConfig networkConfig;
    MqttConfig mqttConfig;
}; // namespace Networking

