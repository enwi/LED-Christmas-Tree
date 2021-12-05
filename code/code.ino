#if defined(ESP8266)
#define FASTLED_ALLOW_INTERRUPTS 0
#endif
#define DEBUG_PRINT

#include <AceButton.h>
#include <FastLED.h>

#include "Constants.h"
#include "Menu.h"
#include "TreeLight.h"

using namespace ace_button;

#if defined(ESP8266) || defined(ESP32)
#include <ESPAsyncWebServer.h>

#include "Networking.h"

AsyncWebServer server(80); /// Webserver for OTA
boolean apMode = false; /// Has AP been enabled (true) or not

void initWifi()
{
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

void toggleWifi()
{
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
}

#else
void initWifi() { }
void toggleWifi() { }
#endif

#if defined(ESP8266)
constexpr uint8_t buttonPin = D2;
#else
constexpr uint8_t buttonPin = 2;
#endif

AceButton button(buttonPin);
TreeLight light;
Menu menu;

void selectBrightness()
{
    // 8 levels of brigthness
    menu.setNumSubSelections(8);
    menu.setSubSelection(light.getBrightnessLevel() - 1);
    menu.setMenuState(Menu::MenuState::brightnessSelect);
}

void updateBrightness()
{
    light.setBrightnessLevel(menu.getSubSelection() + 1);
}

void selectColor()
{
    light.initColorMenu();
}

void handleButton(AceButton*, uint8_t eventType, uint8_t)
{
    menu.handleButton(eventType);
    if (menu.isActive())
    {
        // Only process buttons for menu
        return;
    }
    switch (eventType)
    {
    case AceButton::kEventClicked:
        light.nextEffect();
        break;
    case AceButton::kEventDoubleClicked:
        light.nextSpeed();
        break;
    default:
        break;
    }
#ifdef DEBUG_PRINT
    switch (eventType)
    {
    case AceButton::kEventClicked:
        Serial.println("click");
        break;
    case AceButton::kEventPressed:
        Serial.println("press");
        break;
    case AceButton::kEventReleased:
        Serial.println("release");
        break;
    case AceButton::kEventDoubleClicked:
        Serial.println("double");
        break;
    case AceButton::kEventRepeatPressed:
        Serial.println("repeat");
        break;
    case AceButton::kEventLongPressed:
        Serial.println("long");
        break;
    default:
        break;
    }
#endif
}

void setup()
{
    light.init(menu);
#ifdef DEBUG_PRINT
    Serial.begin(57600);
#endif

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

    initWifi();
    menu.setMainCallback(1, selectBrightness);
    menu.setMainCallback(2, selectColor);
    menu.setMainCallback(3, toggleWifi);
    menu.setBrightnessCallback(updateBrightness);
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
    delay(1);

#ifdef DEBUG_PRINT
    unsigned long t = millis();
    if (t % 1000 == 0)
    {
        Serial.print(t / 1000);
        Serial.print(" - Current effect ");
        Serial.println((int)light.getEffect());
    }
#endif
}
