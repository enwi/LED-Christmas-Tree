//#define FASTLED_ALLOW_INTERRUPTS 0
#include <AceButton.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>

#include "Constants.h"
#include "Menu.h"
#include "Networking.h"
#include "TreeLight.h"


using namespace ace_button;

AsyncWebServer server(80); /// Webserver for OTA
boolean apMode = false; /// Has AP been enabled (true) or not

constexpr uint8_t buttonPin = D2;

AceButton button(buttonPin);
void handleButton(AceButton*, uint8_t eventType, uint8_t);
TreeLight light;
Menu menu;

void setup()
{
    light.init(menu);

    pinMode(buttonPin, INPUT);
    ButtonConfig* buttonConfig = button.getButtonConfig();
    buttonConfig->setEventHandler(handleButton);
    buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterDoubleClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
    buttonConfig->setRepeatPressDelay(1000);
    buttonConfig->setRepeatPressInterval(1000);

    uint8_t mac[6];
    wifi_get_macaddr(STATION_IF, mac);
    sniprintf(deviceMAC, sizeof(deviceMAC), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    server.on(
        "/ota", HTTP_POST, [](AsyncWebServerRequest* request) { request->send(200); }, Networking::handleOTAUpload);

    server.on("/ota", HTTP_GET, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", OTA_INDEX);
        request->send(response);
    });
    server.begin();
}

void loop()
{
    // 1. Check button state:
    //    - Debounce
    //    - Single click / double click detection
    //    - Long hold detection
    // 2. Process button update
    // 3. Update LEDs (if necessary)
    // 4. wait until next update / button poll

    // click: change effect
    // double: speed
    // long 1s: brightness
    // long 2s: color (palette)
    // long 3s: WiFi

    // 1. & 2.
    button.check();

    // 3.
    light.update();
}

void handleButton(AceButton*, uint8_t eventType, uint8_t)
{
    switch (eventType)
    {
    case AceButton::kEventClicked:
        light.nextEffect();
        break;
    case AceButton::kEventDoubleClicked:
        light.nextSpeed();
        break;
    case AceButton::kEventRepeatPressed:
        uint8_t selectedMode = menu.getLongPressMode();
        if (++selectedMode > 3)
        {
            selectedMode = 1;
        }
        menu.setLongPressMode(selectedMode);
        break;
    case AceButton::kEventReleased:
        switch (menu.getLongPressMode())
        {
        case 1:
            break;
        case 2:
            break;
        case 3:
            // Create AP for OTA
            if (apMode)
            {
                WiFi.softAPdisconnect(true);
            }
            else
            {
                char ssid[33] = {};
                sniprintf(ssid, sizeof(ssid), "%s %s", HOSTNAME, deviceMAC);
                Networking::createAP(ssid);
            }
            apMode = !apMode;
            break;
        }
        menu.setLongPressMode(0);
        break;
    }
}
