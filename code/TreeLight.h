#ifndef TREE_LIGHT_H
#define TREE_LIGHT_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#include "Menu.h"
#include "TreeColors.h"
#include "TreeEffects.h"

// Config:
// - brighness
// - color (rgb/palette)
// - effect
// - on/off
// - speed (off,slow,normal,fast)

// LED order:
//       USB
//        0
//    7   8   1
// 6   11 12 9   2
//    5   10   3
//        4

// Improvements:
// - Feedback when effect is changed by button (short flash?)
// - Indicate Wifi on/off when option is selected (different fading animations)
// - Color selection first option in menu, because it is changed more often than brightness

enum class Speed
{
    stopped = 0,
    slow = 1,
    medium = 2,
    fast = 4,
    maxValue // Not a speed
};

class TreeLight
{
public:
    friend class TreeLightView;

    void init(Menu& menu);
    void getStatusJsonString(JsonObject& output);
    static const char* effect_names[];

    void nextEffect();
    void setEffect(EffectType e);
    EffectType getEffectType() const { return currentEffectType; }
    IEffect* getEffect() const { return currentEffect; }
    void nextSpeed();
    void setSpeed(Speed s);
    uint8_t getSpeed() const { return speed; }
    void update();
    void setLED(const uint8_t led, const CRGB color)
    {
        if (led > leds.size())
        {
            return;
        }
        leds[led] = color;
        FastLED.show();
    }
    void resetEffect(bool timerOnly = true);
    void setBrightnessLevel(uint8_t level);
    uint8_t getBrightnessLevel() const { return brightnessLevel; }

    void setLED(const uint8_t start, const uint8_t end, const CRGB color)
    {
        if (start > leds.size() || end > leds.size())
        {
            return;
        }
        leds(start, end) = color;
        FastLED.show();
    }
    void initColorMenu();
    void setColorSelection(uint8_t index) { colors.setSelection(index); }

    const TreeColors& getColors() const { return colors; }
    TreeColors& getColors() { return colors; }

public:
#if defined(ESP8266)
    static constexpr uint8_t pin = D1;
#else
    static constexpr uint8_t pin = 3;
#endif
    static constexpr uint8_t numLeds = 13;

private:
    void runEffect();
    void displayMenu();

private:
    Menu* menu;
    CRGBArray<numLeds> leds;
    CRGBArray<numLeds> ledBackup; // For fade over from different effect
    unsigned long effectTime = 0;
    unsigned long lastUpdate = 0;
    EffectType currentEffectType = EffectType::off;
    IEffect* currentEffect = nullptr;
    IEffect** effectList;
    uint8_t speed = 2;
    uint8_t brightnessLevel = 4;
    unsigned long menuTime = 0;
    TreeColors colors;
};

class TreeLightView
{
public:
    TreeLightView(TreeLight& light) : l(&light) { }

    void resetEffect(bool timerOnly = true) { l->resetEffect(timerOnly); }
    void updateColor() { l->getColors().updateColor(); }
    CRGB firstColor() const { return l->getColors().firstColor(); }
    CRGB secondColor() const { return l->getColors().secondColor(); }
    bool isColorPalette() const { return l->getColors().isColorPalette(); }

    CRGB getPaletteColor(uint8_t mix, bool doBlend = true) const
    {
        return l->getColors().getPaletteColor(mix, doBlend);
    }

private:
    TreeLight* l;
};

#endif
