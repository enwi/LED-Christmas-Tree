#include "Config.h"

#if defined(ESP8266) || defined(ESP32)

constexpr int documentSize = 1024;

NetworkConfig Config::networkConfig;
MqttConfig Config::mqttConfig;

void Config::initConfig()
{
    if (SPIFFS.begin())
    {
        DEBUGLN("Mounted file system");
        if (SPIFFS.exists("/config.json"))
        {

            DEBUGLN("Reading config file");
            File configFile = SPIFFS.open("/config.json", "r");

            if (configFile)
            {
                DEBUGLN("Opened config file");

                StaticJsonDocument<documentSize> json;

                DeserializationError error = deserializeJson(json, configFile);
                if (error)
                {
                    DEBUGLN(F("Failed to read file, using default configuration"));
                    Config::setDefaultConfig();
                }
                else
                {
                    DEBUGLN(F("Successfully loaded config file"));
                }
                networkConfig.fromJson(json["wifi"]);
                mqttConfig.fromJson(json["mqtt"]);
            }
        }
        else
        {
            DEBUGLN("Config file does not exist");
            Config::setDefaultConfig();
            Config::save();
        }
    }
    else
    {
        DEBUGLN("Failed to mount FS");
        Config::setDefaultConfig();
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

void Config::setDefaultConfig()
{
    char ssid[33] = {};
    sniprintf(ssid, sizeof(ssid), "%s %s", HOSTNAME, deviceMAC);
    networkConfig.apSsid = ssid;
}

void Config::save()
{
    DEBUGLN("Writing config file");
    File configFile = SPIFFS.open("/config.json", "w");

    StaticJsonDocument<documentSize> json;
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

#endif

void NetworkConfig::fromJson(const JsonObjectConst& object)
{
    clientEnabled = object["client_enabled"];
    dhcpEnabled = object["client_dhcp_enabled"];
    clientSsid = object["client_ssid"].as<const char*>();
    clientPassword = object["client_password"].as<const char*>();
    clientIp.fromString(object["client_ip"].as<const char*>());
    clientGateway.fromString(object["client_gateway"].as<const char*>());
    clientDns.fromString(object["client_dns"].as<const char*>());
    clientMask.fromString(object["client_mask"].as<const char*>());
    apEnabled = object["ap_enabled"];
    apSsid = object["ap_ssid"].as<const char*>();
    apPassword = object["ap_password"].as<const char*>();
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
}

void MqttConfig::fromJson(const JsonObjectConst& object)
{
    enabled = object["enabled"];
    server = object["server"].as<const char*>();
    port = object["port"];
    id = object["id"].as<const char*>();
    user = object["user"].as<const char*>();
    password = object["password"].as<const char*>();
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
