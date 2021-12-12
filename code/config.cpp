#include "config.h"

StaticJsonDocument<1024> *Config::config;

void Config::initConfig()
{
    if (SPIFFS.begin()) {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json")) {
            
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");

            if (configFile) {
                Serial.println("opened config file");
                // size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                // std::unique_ptr<char[]> buf(new char[size]);

                // configFile.readBytes(buf.get(), size);
                // DynamicJsonBuffer jsonBuffer;

                config = new StaticJsonDocument<1024>;

                DeserializationError error = deserializeJson(*config, configFile);
                if (error)
                    Serial.println(F("Failed to read file, using default configuration"));

                serializeJson(*config, Serial);
                Serial.println();

                // config = jsonBuffer.parseObject(buf.get());
            }
        } else {
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
            (*config)["mqtt"]["server"] = true;
            (*config)["mqtt"]["port"] = 1883;
            (*config)["mqtt"]["id"] = "LedChristmasTree";
            (*config)["mqtt"]["user"] = "";
            (*config)["mqtt"]["password"] = "";
            
            Config::save();
        }
    } else {
        Serial.println("failed to mount FS");
    }
}

void Config::save()
{
    Serial.println("Writing config file");
    File configFile = SPIFFS.open("/config.json", "w");

    if (serializeJson(*config, configFile) == 0) {
        Serial.println(F("Failed to write to file"));
    }
    serializeJson(*config, Serial);
    Serial.println();

    configFile.close();
}