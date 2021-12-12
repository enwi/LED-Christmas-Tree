#if defined(ESP8266) || defined(ESP32)
#include "Networking.h"

const IPAddress Networking::AP_IP = {192,168,4,1};
const IPAddress Networking::AP_NETMASK = {255, 255, 255, 0};

void Networking::initWifi()
{
    Serial.println("Init wifi");

    char ssid[33] = {};
    sniprintf(ssid, sizeof(ssid), "%s %s", HOSTNAME, deviceMAC);

    bool client_enabled = (*Config::config)["wifi"]["client_enabled"];
    if( client_enabled ) {
        Serial.println("Using wifi in client mode");
        WiFi.mode(WIFI_STA);
        String ssid = (*Config::config)["wifi"]["client_ssid"];
        String psk = (*Config::config)["wifi"]["client_password"];
        bool dhcp = (*Config::config)["wifi"]["client_dhcp_enabled"];

        if(!dhcp) {
            IPAddress ip;
            IPAddress mask;
            IPAddress gw;
            IPAddress dns;
            ip.fromString((const char*)((*Config::config)["wifi"]["client_ip"]));
            mask.fromString((const char*)((*Config::config)["wifi"]["client_mask"]));
            gw.fromString((const char*)((*Config::config)["wifi"]["client_gateway"]));
            dns.fromString((const char*)((*Config::config)["wifi"]["client_dns"]));
            Serial.println("Using static ip");
            if (!WiFi.config(ip, gw, mask, dns, dns)) {
                Serial.println("STA Failed to configure");
            }
        } else {
            Serial.println("Using DHCP");
        }

        WiFi.begin(ssid, psk);
        Serial.print("Connecting to WiFi ..");
        uint64_t start = millis();
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print('.');
            delay(1000);
            if(start + 15000 < millis()) {
                (*Config::config)["wifi"]["client_enabled"] = false;
                (*Config::config)["wifi"]["ap_enabled"] = true;
                Config::save();
                Serial.println();
                Serial.println('Failed, enabling AP and rebooting');
                Serial.flush();
                ESP.restart();
            }
        }
        Serial.println("Connected");
    }

    bool ap_enabled = (*Config::config)["wifi"]["ap_enabled"];
    if( ap_enabled ) {
        Serial.println("Using wifi in ap mode");
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(AP_IP, AP_IP, AP_NETMASK);
        String ssid = (*Config::config)["wifi"]["ap_ssid"];
        String psk = (*Config::config)["wifi"]["ap_password"];

        if(psk.length()==0) {
            WiFi.softAP(ssid);
            Serial.println("Sarting open AP");
        } else {
            WiFi.softAP(ssid, psk);
            Serial.println("Sarting protected AP");
        }
    }

}

void Networking::initServer(AsyncWebServer *server, TreeLight *light)
{
    server->on("/ota", HTTP_POST, [](AsyncWebServerRequest* request) { request->send(200); }, Networking::handleOTAUpload);
    
    server->on("/api/status", HTTP_GET, [light](AsyncWebServerRequest* request) { Networking::handleStatusApi(request, light); } );
    server->on("/api/config", HTTP_GET, handleConfigApiGet);
    
    AsyncCallbackJsonWebHandler* handlerSetLeds = new AsyncCallbackJsonWebHandler("/api/set_leds", [light](AsyncWebServerRequest *request, JsonVariant &json) {
        Networking::handleSetLedsApi(request, &json, light);
    });
    server->addHandler(handlerSetLeds);

    AsyncCallbackJsonWebHandler* handlerSetConfig = new AsyncCallbackJsonWebHandler("/api/config", [](AsyncWebServerRequest *request, JsonVariant &json) {
        Networking::handleConfigApiPost(request, &json);
    });
    server->addHandler(handlerSetConfig);

    server->on("/", HTTP_GET,   Networking::handleIndex);
    server->on("/home", HTTP_GET,  Networking::handleIndex);
    server->on("/config", HTTP_GET,  Networking::handleIndex);

    server->begin();
}


void Networking::getStatusJsonString(JsonObject &output)
{
    auto && networking = output.createNestedObject("network");

    networking["mac"] = deviceMAC;

    bool client_enabled = (*Config::config)["wifi"]["client_enabled"];

    auto && wifi_client = networking.createNestedObject("wifi_client");
    wifi_client["status"] = client_enabled ? ("enabled") : "disabled";
    wifi_client["ip"] = WiFi.localIP();
    wifi_client["netmask"] = "0.0.0.0";
    wifi_client["dns"] = "0.0.0.0";

    auto && wifi_ap = networking.createNestedObject("wifi_ap");
    wifi_ap["status"] = client_enabled ? "disabled" : "enabled";
    wifi_ap["ip"] = WiFi.softAPIP();
}


void Networking::handleOTAUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final)
{
    if (!index)
    {
        Serial.printf("UploadStart: %s\n", filename.c_str());
        // calculate sketch space required for the update, for ESP32 use the max constant
#if defined(ESP32)
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
#else
        const uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace))
#endif
        {
            // start with max available size
            Update.printError(Serial);
        }
#if defined(ESP8266)
        Update.runAsync(true);
#endif
    }

    if (len)
    {
        Update.write(data, len);
    }

    // if the final flag is set then this is the last frame of data
    if (final)
    {
        if (Update.end(true))
        {
            // true to set the size to the current progress
            Serial.printf("Update Success: %ub written\nRebooting...\n", index + len);
            ESP.restart();
        }
        else
        {
            Update.printError(Serial);
        }
    }
}

void Networking::handleIndex(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), build_html_gz_start, build_html_gz_size);
    response->addHeader(F("Content-Encoding"), F("gzip"));
    request->send(response);
}

void Networking::handleStatusApi(AsyncWebServerRequest *request, TreeLight *light) {
    DynamicJsonDocument output(3000);

    auto && obj = output.to<JsonObject>();
  
    obj["uptime"] = millis()/1000;
    obj["heap_free"] = ESP.getFreeHeap();

    Networking::getStatusJsonString(obj);
    Mqtt::getStatusJsonString(obj);
    light->getStatusJsonString(obj);

    String buffer;
    buffer.reserve(512);
    serializeJson(output, buffer);

    request->send(200, "application/json", buffer);
}

void Networking::handleConfigApiGet(AsyncWebServerRequest *request) {
    String buffer;
    buffer.reserve(512);
    serializeJson(*Config::config, buffer);

    request->send(200, "application/json", buffer);
}


void Networking::handleConfigApiPost(AsyncWebServerRequest *request, JsonVariant *json) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");

    Serial.print("Received new config: ");
    serializeJson(*json, Serial);
    Serial.println();

    JsonObject && data = json->as<JsonObject>();
    
    (*Config::config)["wifi"]["client_enabled"] =      (bool)data["wifi"]["client_enabled"];
    (*Config::config)["wifi"]["client_dhcp_enabled"] = (bool)data["wifi"]["client_dhcp_enabled"];
    (*Config::config)["wifi"]["client_ssid"] =     String((const char*)data["wifi"]["client_ssid"]);
    (*Config::config)["wifi"]["client_password"] = String((const char*)data["wifi"]["client_password"]);
    (*Config::config)["wifi"]["client_ip"] =       String((const char*)data["wifi"]["client_ip"]);
    (*Config::config)["wifi"]["client_mask"] =     String((const char*)data["wifi"]["client_mask"]);
    (*Config::config)["wifi"]["client_gateway"] =  String((const char*)data["wifi"]["client_gateway"]);
    (*Config::config)["wifi"]["client_dns"] =      String((const char*)data["wifi"]["client_dns"]);

    (*Config::config)["wifi"]["ap_enabled"] =  (bool)data["wifi"]["ap_enabled"];
    (*Config::config)["wifi"]["ap_ssid"] =     String((const char*)data["wifi"]["ap_ssid"]);
    (*Config::config)["wifi"]["ap_password"] = String((const char*)data["wifi"]["ap_password"]);


    (*Config::config)["mqtt"]["enabled"] =  (bool)data["mqtt"]["enabled"];
    (*Config::config)["mqtt"]["server"] =   String((const char*)data["mqtt"]["server"]);
    (*Config::config)["mqtt"]["port"] =     (uint16_t)data["mqtt"]["port"];
    (*Config::config)["mqtt"]["id"] =       String((const char*)data["mqtt"]["id"]);
    (*Config::config)["mqtt"]["user"] =     String((const char*)data["mqtt"]["user"]);
    (*Config::config)["mqtt"]["password"] = String((const char*)data["mqtt"]["password"]);

    Config::save();

    response->print("OK");
    request->send(response);

    delay(1000);
    ESP.restart();
}


void Networking::handleSetLedsApi(AsyncWebServerRequest *request, JsonVariant *json, TreeLight *light) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    JsonObject && data = json->as<JsonObject>();
    light->setBrightnessLevel(data["brightness"]);
    light->setSpeed(static_cast<Speed>((uint8_t)data["speed"]));
    bool enabled = data["enabled"];
    if(enabled) {
        light->setEffect(EffectType::rainbowHorizontal);
    } else {
        light->setEffect(EffectType::off);
    }
    response->print("OK");
    request->send(response);
}



#endif
