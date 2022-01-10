#if defined(ESP8266) || defined(ESP32)
#include "Networking.h"

#include "webui/cpp/build.html.gz.h"

const IPAddress Networking::AP_IP = {192, 168, 4, 1};
const IPAddress Networking::AP_NETMASK = {255, 255, 255, 0};
DNSServer Networking::dnsServer = {};

void Networking::initWifi()
{
    DEBUGLN("Init wifi");

    char ssid[33] = {};
    sniprintf(ssid, sizeof(ssid), "%s %s", HOSTNAME, deviceMAC);

    NetworkConfig& wifi = Config::getNetworkConfig();

    if (wifi.apEnabled && wifi.clientEnabled)
    {
        WiFi.mode(WIFI_AP_STA);
    }
    else if (wifi.apEnabled)
    {
        WiFi.mode(WIFI_AP);
    }
    else if (wifi.clientEnabled)
    {
        WiFi.mode(WIFI_STA);
    }

    if (wifi.clientEnabled)
    {
        DEBUGLN("Using wifi in client mode");

        if (!wifi.dhcpEnabled)
        {
            DEBUGLN("Using static ip");
            if (!WiFi.config(wifi.clientIp, wifi.clientGateway, wifi.clientMask, wifi.clientDns))
            {
                DEBUGLN("STA Failed to configure");
            }
        }
        else
        {
            DEBUGLN("Using DHCP");
        }

        WiFi.begin(wifi.clientSsid, wifi.clientPassword);
        DEBUG("Connecting to WiFi ..");
        uint64_t start = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
            DEBUG('.');
            delay(1000);
            if (start + 15000 < millis())
            {
                wifi.clientEnabled = false;
                wifi.apEnabled = true;
                Config::save();
                DEBUGLN();
                DEBUGLN('Failed, enabling AP and rebooting');
                Serial.flush();
                ESP.restart();
            }
        }
        DEBUG("Connected: ");
        DEBUGLN(WiFi.localIP());

        WiFi.setAutoConnect(true);
        WiFi.setAutoReconnect(true);
    }

    if (wifi.apEnabled)
    {
        DEBUGLN("Using wifi in ap mode");
        WiFi.softAPConfig(AP_IP, AP_IP, AP_NETMASK);

        if (wifi.apPassword.length() == 0)
        {
            WiFi.softAP(wifi.apSsid);
            DEBUGLN("Starting open AP");
        }
        else
        {
            WiFi.softAP(wifi.apSsid, wifi.apPassword);
            DEBUGLN("Starting protected AP");
        }

        // captive portal
        DEBUGLN("Starting DNS server");
        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(53, "*", WiFi.softAPIP());
    }
    else
    {
        // captive portal
        DEBUGLN("Stopping DNS server");
        dnsServer.stop();
    }
}

void Networking::initServer(AsyncWebServer* server, TreeLight* light)
{
    server->on(
        "/ota", HTTP_POST, [](AsyncWebServerRequest* request) { request->send(200); }, Networking::handleOTAUpload);

    server->on("/api/status", HTTP_GET,
        [light](AsyncWebServerRequest* request) { Networking::handleStatusApi(request, light); });
    server->on("/api/config", HTTP_GET, handleConfigApiGet);

    AsyncCallbackJsonWebHandler* handlerSetLeds
        = new AsyncCallbackJsonWebHandler("/api/set_leds", [light](AsyncWebServerRequest* request, JsonVariant& json) {
              Networking::handleSetLedsApi(request, &json, light);
          });
    server->addHandler(handlerSetLeds);

    AsyncCallbackJsonWebHandler* handlerSetConfig = new AsyncCallbackJsonWebHandler("/api/config",
        [](AsyncWebServerRequest* request, JsonVariant& json) { Networking::handleConfigApiPost(request, &json); });
    server->addHandler(handlerSetConfig);

    server->on("/", HTTP_GET, Networking::handleIndex);
    server->on("/home", HTTP_GET, Networking::handleIndex);
    server->on("/config", HTTP_GET, Networking::handleIndex);

    // captive portal
    auto handleCaptivePortal = [](AsyncWebServerRequest* request) { captivePortal(request); };
    // Android captive portal. Maybe not needed. Might be handled by notFound handler.
    server->on("/generate_204", HTTP_GET, handleCaptivePortal);
    // Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    server->on("/fwlink", HTTP_GET, handleCaptivePortal);
    server->onNotFound(handleCaptivePortal);

    server->begin();
}

void Networking::getStatusJsonString(JsonObject& output)
{
    auto&& networking = output.createNestedObject("network");

    networking["mac"] = deviceMAC;

    bool client_enabled = Config::getNetworkConfig().clientEnabled;

    auto&& wifi_client = networking.createNestedObject("wifi_client");
    wifi_client["status"] = client_enabled ? (WiFi.isConnected() ? "connected" : "enabled") : "disabled";
    wifi_client["ip"] = WiFi.localIP();
    wifi_client["netmask"] = WiFi.subnetMask();
    wifi_client["dns"] = WiFi.dnsIP();

    auto&& wifi_ap = networking.createNestedObject("wifi_ap");
    wifi_ap["status"] = client_enabled ? "disabled" : "enabled";
    wifi_ap["ip"] = WiFi.softAPIP();
}

void Networking::handleOTAUpload(
    AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final)
{
    if (!index)
    {
        DEBUGLN("UploadStart");
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
            DEBUGLN("Update Success, \nRebooting...");
            ESP.restart();
        }
        else
        {
            Update.printError(Serial);
        }
    }
}

void Networking::handleIndex(AsyncWebServerRequest* request)
{
    AsyncWebServerResponse* response
        = request->beginResponse_P(200, F("text/html"), build_html_gz_start, build_html_gz_size);
    response->addHeader(F("Content-Encoding"), F("gzip"));
    request->send(response);
}

void Networking::handleStatusApi(AsyncWebServerRequest* request, TreeLight* light)
{
    DynamicJsonDocument output(3000);

    auto&& obj = output.to<JsonObject>();

    obj["uptime"] = millis() / 1000;
    obj["heap_free"] = ESP.getFreeHeap();

    Networking::getStatusJsonString(obj);
    Mqtt::getStatusJsonString(obj);
    light->getStatusJsonString(obj);

    String buffer;
    buffer.reserve(512);
    serializeJson(output, buffer);

    request->send(200, "application/json", buffer);
}

void Networking::handleConfigApiGet(AsyncWebServerRequest* request)
{
    String buffer;
    buffer.reserve(512);
    StaticJsonDocument<1024> document;
    Config::createJson(document);
    // TODO: Remove passwords
    serializeJson(document, buffer);

    request->send(200, "application/json", buffer);
}

void Networking::handleConfigApiPost(AsyncWebServerRequest* request, JsonVariant* json)
{
    AsyncResponseStream* response = request->beginResponseStream("text/html");

    DEBUG("Received new config");

    JsonObject&& data = json->as<JsonObject>();

    NetworkConfig& wifi = Config::getNetworkConfig();
    wifi.fromJson(data["wifi"]);

    MqttConfig& mqtt = Config::getMqttConfig();
    mqtt.fromJson(data["mqtt"]);

    Config::save();

    response->print("OK");
    request->send(response);

    // TODO: Check whether config changed and if restard is needed
    delay(1000);
    ESP.restart();
}

void Networking::handleSetLedsApi(AsyncWebServerRequest* request, JsonVariant* json, TreeLight* light)
{
    AsyncResponseStream* response = request->beginResponseStream("text/html");
    JsonObject&& data = json->as<JsonObject>();
    light->setBrightnessLevel(data["brightness"]);
    light->setSpeed(static_cast<Speed>((uint8_t)data["speed"]));
    light->setEffect(static_cast<EffectType>((uint8_t)data["effect"]));
    light->setColorSelection((uint8_t)data["color"]);
    response->print("OK");
    request->send(response);
}

bool Networking::isIp(const String& str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9'))
        {
            return false;
        }
    }
    return true;
}

void Networking::update()
{
    // handle DNS
    dnsServer.processNextRequest();
}

bool Networking::captivePortal(AsyncWebServerRequest* request)
{
    if (ON_STA_FILTER(request))
    {
        // DEBUGLN(F("Captive STA Filter"));
        return false; // only serve captive portal in AP mode
    }
    if (!request->hasHeader("Host"))
    {
        // DEBUGLN(F("Captive Host header missing"));
        return false;
    }
    const String hostHeader = request->getHeader("Host")->value();
    if (isIp(hostHeader) || hostHeader.indexOf(HOSTNAME) >= 0)
    {
        // DEBUG(F("Captive Host Filter: "));
        // DEBUGLN(hostHeader);
        return false;
    }

    DEBUGLN(F("Captive portal"));
    AsyncWebServerResponse* response = request->beginResponse(302);
    response->addHeader(F("Location"), F("http://192.168.4.1"));
    request->send(response);
    return true;
}

#endif
