#if defined(ESP8266)
#define FASTLED_ALLOW_INTERRUPTS 0
#endif

#include <AceButton.h>
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>

#include "Config.h"
#include "Constants.h"
#include "Menu.h"
#include "Mqtt.h"
#include "Networking.h"
#include "TreeLight.h"

#if defined(ESP32)
#include <esp_wifi.h>
#endif

using namespace ace_button;

#if defined(ESP8266)
constexpr uint8_t buttonPin = D2;
#else
constexpr uint8_t buttonPin = 2;
#endif

AceButton button(buttonPin);
TreeLight light;
Menu menu;
Config config;
Networking networking {config};
bool wifiEnabled = false;

void getMacAddress(uint8_t (&mac)[6])
{
#if defined(ESP32)
    esp_wifi_get_mac(WIFI_IF_STA, mac);
#else
    wifi_get_macaddr(STATION_IF, mac);
#endif
}

void init_config()
{
    uint8_t mac[6] = {};
    getMacAddress(mac);
    sniprintf(deviceMAC, sizeof(deviceMAC), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    config.initConfig();
    DEBUGLN(WiFi.getMode());
    DEBUGLN(WiFi.getAutoConnect());
    if (networking.shouldEnableWifiOnStartup())
    {
        DEBUGLN("Wifi enabled");
        networking.initOrResume(light);
        wifiEnabled = true;
    }
    else
    {
        DEBUGLN("Wifi disabled");
    }
    EffectConfig& effectConfig = config.getEffectConfig();
    light.setBrightnessLevel(effectConfig.brightnessLevel);
    light.setColorSelection(effectConfig.colorSelection);
    light.setSpeed((Speed)effectConfig.speed);
    light.setEffect(effectConfig.currentEffectType);
}
void toggle_wifi()
{
    if (!wifiEnabled)
    {
        wifiEnabled = true;
        networking.initOrResume(light);
    }
    else
    {
        wifiEnabled = false;
        networking.stop();
    }
}

void save_effect()
{
    EffectConfig& effectConfig = config.getEffectConfig();
    bool changed = false;
    if (effectConfig.brightnessLevel != light.getBrightnessLevel())
    {
        effectConfig.brightnessLevel = light.getBrightnessLevel();
        changed |= true;
    }
    if (effectConfig.colorSelection != light.getColors().getSelection())
    {
        effectConfig.colorSelection = light.getColors().getSelection();
        changed |= true;
    }
    if (effectConfig.speed != (uint8_t)light.getSpeed())
    {
        effectConfig.speed = (uint8_t)light.getSpeed();
        changed |= true;
    }
    if (effectConfig.currentEffectType != light.getEffectType())
    {
        effectConfig.currentEffectType = light.getEffectType();
        changed |= true;
    }

    if (changed)
    {
        config.saveEffect();
    }
}

void selectBrightness()
{
    // 8 levels of brigthness
    menu.setNumSubSelections(8);
    menu.setMenuState(Menu::MenuState::brightnessSelect);
    menu.setSubSelection(light.getBrightnessLevel() - 1);
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
    if (menu.handleButton(eventType))
    {
        return;
    }
    switch (eventType)
    {
    case AceButton::kEventClicked:
        DEBUGLN("Button clicked");
        light.nextEffect();
        break;
    case AceButton::kEventPressed:
        DEBUGLN("Button pressed");
        break;
    case AceButton::kEventReleased:
        DEBUGLN("Button released");
        light.nextEffect();
        break;
    case AceButton::kEventDoubleClicked:
        DEBUGLN("Button double clicked");
        light.nextSpeed();
        break;
    case AceButton::kEventRepeatPressed:
        DEBUGLN("Button repeat");
        break;
    case AceButton::kEventLongPressed:
        DEBUGLN("Button longpress");
        break;
    default:
        break;
    }
}

void setup()
{
    light.init(menu);
#ifdef DEBUG_PRINT
    Serial.begin(57600);
    DEBUGLN("Debug output enabled");
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

    init_config();

    menu.setMainCallback(1, selectBrightness);
    menu.setMainCallback(2, selectColor);
    menu.setMainCallback(3, save_effect);
    menu.setMainCallback(4, toggle_wifi);
    menu.setBrightnessCallback(updateBrightness);
}

#ifdef DEBUG_PRINT
unsigned long printTime = 0;
#endif

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

#if defined(ESP8266) || defined(ESP32)
    EVERY_N_SECONDS(1)
    {
        networking.update();
    }
#endif

#ifdef DEBUG_PRINT
    unsigned long t = millis();
    if (t - printTime > 1000)
    {
        printTime = t;
        DEBUG(t / 1000);
        DEBUG(" - Current effect ");
        DEBUG((int)light.getEffectType());
        IEffect* e = light.getEffect();
        DEBUG(" ");
        if (e)
        {
            DEBUG(e->getName());
        }
        else
        {
            DEBUG("NULL");
        }
        DEBUG(", FPS: ");
        DEBUGLN(FastLED.getFPS());
    }
#endif
}
