#pragma once

#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

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

    /// @brief Verify that the object can be parsed
    /// @returns true if fromJson can be executed
    bool verify(const JsonObjectConst& object) const;
    void fromJson(const JsonObjectConst& object);
    void toJson(JsonObject& object) const;

    /// @brief Update all fields in object, if possible
    /// @returns true when any value was changed
    bool tryUpdate(const JsonObjectConst& object);
};

struct MqttConfig
{
    bool enabled = false;
    String server;
    unsigned int port = 1883;
    String id = "LedChristmasTree";
    String user;
    String password;

    /// @brief Verify that the object can be parsed
    /// @returns true if fromJson can be executed
    bool verify(const JsonObjectConst& object) const;
    void fromJson(const JsonObjectConst& object);
    void toJson(JsonObject& object) const;

    /// @brief Update all fields in object, if possible
    /// @returns true when any value was changed
    bool tryUpdate(const JsonObjectConst& object);
};

// These effect types have to match the order in createEffects() in the cpp file
enum class EffectType
{
    off,
    solid,
    twoColorChange,
    gradientHorizontal,
    gradientVertical,
    rainbowHorizontal,
    rainbowVertical,
    runningLight,
    twinkleFox,
    cycling,
    maxValue // Not an effect, number of valid effects
};

struct EffectConfig
{
    uint8_t speed = 2;
    uint8_t brightnessLevel = 4;
    EffectType currentEffectType = EffectType::off;
    uint8_t colorSelection = 0;

    /// @brief Verify that the object can be parsed
    /// @returns true if fromJson can be executed
    bool verify(const JsonObjectConst& object) const;
    void fromJson(const JsonObjectConst& object);
    void toJson(JsonObject& object) const;

    /// @brief Update all fields in object, if possible
    /// @returns true when any value was changed
    bool tryUpdate(const JsonObjectConst& object);
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
    void readConfig();
    void readEffect();

private:
    NetworkConfig networkConfig;
    MqttConfig mqttConfig;
    EffectConfig effectConfig;
}; // namespace Networking
