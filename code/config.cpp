#include "Config.h"

constexpr int documentSizeConfig = 1024;
constexpr int documentSizeEffect = 128;

namespace
{
    /// @brief Helper to update a value from json and also check if it was changed
    /// @param object Json object, maybe null
    /// @param field Field name of the value
    /// @param val Reference to the variable that should be changed if possible
    /// @returns true if the value in object was valid and different from val
    template <typename T>
    bool updateField(const JsonObjectConst& object, const char* field, T& val)
    {
        T newValue = object[field] | val;
        if (newValue != val)
        {
            val = newValue;
            return true;
        }
        return false;
    }
    template <>
    bool updateField<String>(const JsonObjectConst& object, const char* field, String& val)
    {
        const char* newValue = object[field] | val.c_str();
        if (val != newValue)
        {
            val = newValue;
            return true;
        }
        return false;
    }
    template <>
    bool updateField<IPAddress>(const JsonObjectConst& object, const char* field, IPAddress& val)
    {
        const char* newValue = object[field] | "";
        IPAddress newIp;
        if (!newIp.fromString(newValue))
        {
            return false;
        }
        if (newIp != val)
        {
            val = newIp;
            return true;
        }
        return false;
    }
} // namespace

void Config::initConfig()
{
    if (SPIFFS.begin())
    {
        DEBUGLN("Mounted file system");
        if (SPIFFS.exists("/config.json"))
        {
            readConfig();
        }
        else
        {
            DEBUGLN("Config file does not exist");
            setDefaultConfig();
            saveConfig();
        }
        if (SPIFFS.exists("/effect.json"))
        {
            readEffect();
        }
        else
        {
            DEBUGLN("Effect file does not exist");
            // No need to create a default file, because the EffectConfig is initialized to default values
        }
    }
    else
    {
        DEBUGLN("Failed to mount FS");
        setDefaultConfig();
    }
}

NetworkConfig& Config::getNetworkConfig()
{
    return networkConfig;
}

MqttConfig& Config::getMqttConfig()
{
    return mqttConfig;
}

EffectConfig& Config::getEffectConfig()
{
    return effectConfig;
}

void Config::setDefaultConfig()
{
    char ssid[33] = {};
    sniprintf(ssid, sizeof(ssid), "%s %s", HOSTNAME, deviceMAC);
    networkConfig.apSsid = ssid;
}

void Config::saveConfig()
{
    DEBUGLN("Writing config file");
    File configFile = SPIFFS.open("/config.json", "w");

    StaticJsonDocument<documentSizeConfig> json;
    createJson(json);

    if (serializeJson(json, configFile) == 0)
    {
        DEBUGLN(F("Failed to write to file"));
    }
    else
    {
        DEBUGLN(F("Successfully updated config."));
    }
    configFile.close();
}

void Config::createJson(JsonDocument& output)
{
    JsonObject wifi = output.createNestedObject("wifi");
    networkConfig.toJson(wifi);
    JsonObject mqtt = output.createNestedObject("mqtt");
    mqttConfig.toJson(mqtt);
}

void Config::saveEffect()
{
    DEBUGLN("Writing effect file");
    File effectFile = SPIFFS.open("/effect.json", "w");

    StaticJsonDocument<documentSizeEffect> json;
    JsonObject o = json.to<JsonObject>();
    effectConfig.toJson(o);

    if (serializeJson(json, effectFile) == 0)
    {
        DEBUGLN(F("Failed to write to file"));
    }
    else
    {
        DEBUGLN(F("Successfully updated effect."));
    }

    effectFile.close();
}

void Config::readConfig()
{
    DEBUGLN("Reading config file");
    File configFile = SPIFFS.open("/config.json", "r");

    if (configFile)
    {
        DEBUGLN("Opened config file");

        StaticJsonDocument<documentSizeConfig> json;

        DeserializationError error = deserializeJson(json, configFile);
        if (error)
        {
            DEBUGLN(F("Failed to read file, using default configuration"));
            setDefaultConfig();
            return;
        }
        else
        {
            DEBUGLN(F("Successfully loaded config file"));
        }
        configFile.close();
        const auto& jWifi = json["wifi"];
        const auto& jMqtt = json["mqtt"];
        if (networkConfig.verify(jWifi) && mqttConfig.verify(jMqtt))
        {
            networkConfig.fromJson(jWifi);
            mqttConfig.fromJson(jMqtt);
        }
        else
        {
            DEBUGLN(F("Invalid file contents"));
            setDefaultConfig();
            if (networkConfig.tryUpdate(jWifi) || mqttConfig.tryUpdate(jMqtt))
            {
                DEBUGLN(F("Read partial data from config.json"));
                // Check wifi configuration for backwards compatibility
                if (!jWifi.containsKey("wifi_enabled") && networkConfig.clientEnabled)
                {
                    networkConfig.wifiEnabled = true;
                }
            }
            saveConfig();
        }
    }
}

void Config::readEffect()
{
    DEBUGLN("Reading effect file");
    File effectFile = SPIFFS.open("/effect.json", "r");

    if (effectFile)
    {
        DEBUGLN("Opened effect file");
        StaticJsonDocument<documentSizeEffect> json;
        DeserializationError error = deserializeJson(json, effectFile);
        if (error)
        {
            DEBUGLN(F("Failed to read file, using default effect config"));
            effectConfig = {};
        }
        else
        {
            DEBUGLN(F("Successfully loaded effect file"));
        }
        effectFile.close();
        const auto& obj = json.as<JsonObject>();
        if (effectConfig.verify(obj))
        {
            effectConfig.fromJson(obj);
        }
        else
        {
            DEBUGLN(F("Invalid file contents"));
            effectConfig = {};
            if (effectConfig.tryUpdate(obj))
            {
                DEBUGLN(F("Read partial data from effect.json"));
            }
            saveEffect();
        }
    }
}

bool NetworkConfig::verify(const JsonObjectConst& object) const
{
    return object["client_enabled"].is<bool>() && object["client_dhcp_enabled"].is<bool>()
        && object["client_ssid"].is<const char*>() && object["client_password"].is<const char*>()
        && object["client_gateway"].is<const char*>() && object["client_dns"].is<const char*>()
        && object["client_mask"].is<const char*>() && object["ap_enabled"].is<bool>()
        && object["ap_ssid"].is<const char*>() && object["ap_password"].is<const char*>()
        && object["wifi_enabled"].is<bool>();
}

void NetworkConfig::fromJson(const JsonObjectConst& object)
{
    constexpr const char* emptyString = "";
    clientEnabled = object["client_enabled"];
    dhcpEnabled = object["client_dhcp_enabled"];
    clientSsid = object["client_ssid"] | emptyString;
    clientPassword = object["client_password"] | emptyString;
    clientIp.fromString(object["client_ip"] | emptyString);
    clientGateway.fromString(object["client_gateway"] | emptyString);
    clientDns.fromString(object["client_dns"] | emptyString);
    clientMask.fromString(object["client_mask"] | emptyString);
    apEnabled = object["ap_enabled"];
    apSsid = object["ap_ssid"] | emptyString;
    apPassword = object["ap_password"] | emptyString;
    wifiEnabled = object["wifi_enabled"];
}

void NetworkConfig::toJson(JsonObject& object) const
{
    object["client_enabled"] = clientEnabled;
    object["client_dhcp_enabled"] = dhcpEnabled;
    object["client_ssid"] = clientSsid;
    object["client_password"] = clientPassword;
    object["client_ip"] = clientIp.toString();
    object["client_gateway"] = clientGateway.toString();
    object["client_dns"] = clientDns.toString();
    object["client_mask"] = clientMask.toString();
    object["ap_enabled"] = apEnabled;
    object["ap_ssid"] = apSsid;
    object["ap_password"] = apPassword;
    object["wifi_enabled"] = wifiEnabled;
}

bool NetworkConfig::tryUpdate(const JsonObjectConst& object)
{
    if (object.isNull())
    {
        return false;
    }
    bool changed = false;
    changed |= updateField(object, "client_enabled", clientEnabled);
    changed |= updateField(object, "client_dhcp_enabled", dhcpEnabled);
    changed |= updateField(object, "client_ssid", clientSsid);
    changed |= updateField(object, "client_password", clientPassword);
    changed |= updateField(object, "client_ip", clientIp);
    changed |= updateField(object, "client_gateway", clientGateway);
    changed |= updateField(object, "client_dns", clientDns);
    changed |= updateField(object, "client_mask", clientMask);
    changed |= updateField(object, "ap_enabled", apEnabled);
    changed |= updateField(object, "ap_ssid", apSsid);
    changed |= updateField(object, "ap_password", apPassword);
    changed |= updateField(object, "wifi_enabled", wifiEnabled);

    return changed;
}

bool MqttConfig::verify(const JsonObjectConst& object) const
{
    return object["enabled"].is<bool>() && object["server"].is<const char*>() && object["port"].is<unsigned int>()
        && object["id"].is<const char*>() && object["user"].is<const char*>() && object["password"].is<const char*>();
}

void MqttConfig::fromJson(const JsonObjectConst& object)
{
    constexpr const char* emptyString = "";
    enabled = object["enabled"];
    server = object["server"] | emptyString;
    port = object["port"];
    id = object["id"] | emptyString;
    user = object["user"] | emptyString;
    password = object["password"] | emptyString;
}

void MqttConfig::toJson(JsonObject& object) const
{
    object["enabled"] = enabled;
    object["server"] = server;
    object["port"] = port;
    object["id"] = id;
    object["user"] = user;
    object["password"] = password;
}

bool MqttConfig::tryUpdate(const JsonObjectConst& object)
{
    if (object.isNull())
    {
        return false;
    }

    bool changed = false;
    changed |= updateField(object, "enabled", enabled);
    changed |= updateField(object, "server", server);
    changed |= updateField(object, "port", port);
    changed |= updateField(object, "id", id);
    changed |= updateField(object, "user", user);
    changed |= updateField(object, "password", password);

    return changed;
}

bool EffectConfig::verify(const JsonObjectConst& object) const
{
    return object["speed"].is<uint8_t>() && object["brightness"].is<uint8_t>() && object["effect"].is<int>()
        && object["color"].is<uint8_t>();
}

void EffectConfig::fromJson(const JsonObjectConst& object)
{
    speed = object["speed"];
    brightnessLevel = object["brightness"];
    currentEffectType = (EffectType)object["effect"].as<int>();
    colorSelection = object["color"];
}

void EffectConfig::toJson(JsonObject& object) const
{
    object["speed"] = (int)speed;
    object["brightness"] = (int)brightnessLevel;
    object["effect"] = (int)currentEffectType;
    object["color"] = (int)colorSelection;
}

bool EffectConfig::tryUpdate(const JsonObjectConst& object)
{
    if (object.isNull())
    {
        return false;
    }

    bool changed = false;
    changed |= updateField(object, "speed", speed);
    changed |= updateField(object, "brightness", brightnessLevel);
    int et = (int)currentEffectType;
    changed |= updateField(object, "effect", et);
    currentEffectType = (EffectType)et;
    changed |= updateField(object, "color", colorSelection);
    return changed;
}