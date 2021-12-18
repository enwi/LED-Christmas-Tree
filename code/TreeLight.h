#ifndef TREE_EFFECT_H
#define TREE_EFFECT_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#include "Menu.h"

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

enum class EffectType
{
    off,
    solid,
    twoColorChange,
    gradientHorizontal,
    gradientVertical,
    rainbowHorizontal,
    rainbowVertical,
    runningLight,
    twinkleFox,
    maxValue // Not an effect, number of valid effects
};

enum class Speed
{
    stopped = 0,
    slow = 1,
    medium = 2,
    fast = 4,
    maxValue // Not a speed
};

class IEffect
{
public:
    struct EffectControl
    {
        bool allowAutoColorChange = false; // Color can change after this effect pass
        bool fadeOver = true; // Fade over from color of last effect to current effect color
    };

public:
    virtual ~IEffect() = default;

    virtual void reset() {};
    virtual EffectControl runEffect(class TreeLightView& lights, CRGBSet& leds, unsigned long effectTime) = 0;
    virtual const char* getName() const = 0;
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
    EffectType getEffect() const { return currentEffectType; }
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
    void resetEffect();
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
    void setColorSelection(uint8_t index);

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
    void updateColor();
    bool isColorPalette() const;
    CRGB getPaletteColor(uint8_t mix, bool doBlend = true) const;

    // returns nullptr if selection is not a palette
    static TProgmemRGBPalette16* getPaletteSelection(uint8_t i);

private:
    Menu* menu;
    CRGBArray<numLeds> leds;
    CRGBArray<numLeds> ledBackup; // For fade over from different effect
    unsigned long effectTime = 0;
    unsigned long lastUpdate = 0;
    EffectType currentEffectType = EffectType::off;
    IEffect* currentEffect = nullptr;
    IEffect** effectList;
    CRGB currentColor = CRGB(0, 0xA0, 0xFF);
    CRGB color2 = CRGB(0, 0x40, 0xFF);
    CRGBPalette16 currentPalette {CRGB::Black};
    uint8_t speed = 2;
    uint8_t brightnessLevel = 4;
    unsigned long menuTime = 0;
    uint8_t colorSelection = 0;
};

class TreeLightView
{
public:
    TreeLightView(TreeLight& light) : l(&light) { }

    void resetEffect() { l->resetEffect(); }
    void updateColor() { l->updateColor(); }
    CRGB firstColor() const { return l->currentColor; }
    CRGB secondColor() const { return l->color2; }
    bool isColorPalette() const { return l->isColorPalette(); }

    CRGB getPaletteColor(uint8_t mix, bool doBlend = true) const { return l->getPaletteColor(mix, doBlend); }

private:
    TreeLight* l;
};

#endif
