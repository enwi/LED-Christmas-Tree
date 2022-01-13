#pragma once

#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

#include "Constants.h"
#include "TreeEffects.h"

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

struct EffectConfig
{
    uint8_t speed = 2;
    uint8_t brightnessLevel = 4;
    EffectType currentEffectType = EffectType::off;
    uint8_t colorSelection = 0;

    void fromJson(const JsonObjectConst& object);
    void toJson(JsonObject& object) const;
};

class Config
{
public:
    void initConfig();
    NetworkConfig& getNetworkConfig();
    MqttConfig& getMqttConfig();
    EffectConfig& getEffectConfig();
    void setDefaultConfig();
    void saveConfig();
    void createJson(JsonDocument& output);

    void saveEffect();

private:
    NetworkConfig networkConfig;
    MqttConfig mqttConfig;
    EffectConfig effectConfig;
}; // namespace Networking
