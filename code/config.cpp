#include "Config.h"

#if defined(ESP8266) || defined(ESP32)

StaticJsonDocument<1024>* Config::config;

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

                config = new StaticJsonDocument<1024>;

                DeserializationError error = deserializeJson(*config, configFile);
                if (error)
                {
                    DEBUGLN(F("Failed to read file, using default configuration"));
                    Config::setDefaultConfig();
                }
                else
                {
                    DEBUGLN(F("Successfully loaded config file"));
                }
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

void Config::setDefaultConfig()
{
    config = new StaticJsonDocument<1024>;

    char ssid[33] = {};
    sniprintf(ssid, sizeof(ssid), "%s %s", HOSTNAME, deviceMAC);

    (*config)["wifi"]["client_enabled"] = false;
    (*config)["wifi"]["client_dhcp_enabled"] = true;
    (*config)["wifi"]["client_ssid"] = "YourWifi";
    (*config)["wifi"]["client_password"] = "inputyourown";
    (*config)["wifi"]["client_ip"] = "0.0.0.0";
    (*config)["wifi"]["client_mask"] = "0.0.0.0";
    (*config)["wifi"]["client_gateway"] = "0.0.0.0";
    (*config)["wifi"]["client_dns"] = "0.0.0.0";
    (*config)["wifi"]["ap_enabled"] = true;
    (*config)["wifi"]["ap_ssid"] = ssid;
    (*config)["wifi"]["ap_password"] = "";

    (*config)["mqtt"]["enabled"] = false;
    (*config)["mqtt"]["server"] = "";
    (*config)["mqtt"]["port"] = 1883;
    (*config)["mqtt"]["id"] = "LedChristmasTree";
    (*config)["mqtt"]["user"] = "";
    (*config)["mqtt"]["password"] = "";
}

void Config::save()
{
    DEBUGLN("Writing config file");
    File configFile = SPIFFS.open("/config.json", "w");

    if (serializeJson(*config, configFile) == 0)
    {
        DEBUGLN(F("Failed to write to file"));
    }
#ifdef DEBUG_PRINT
    serializeJson(*config, Serial);
#endif
    DEBUGLN();

    configFile.close();
}

#endif