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

class TreeLight
{
public:
    void init(Menu& menu);
    void getStatusJsonString(JsonObject& output);
    static const char* effect_names[];

    void nextEffect();
    void setEffect(EffectType e);
    EffectType getEffect() const { return currentEffect; }
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

    void effectTwoColorChange();
    bool effectRunningLight();
    void effectGradientHorizontal(bool& doFadeIn);
    void effectGradientVertical(bool& doFadeIn);
    void effectRainbowHorizontal();
    void effectRainbowVertical();
    void effectTwinkleFox();
    CRGB computeTwinkle(uint32_t clock, uint8_t salt);

    // returns nullptr if selection is not a palette
    static TProgmemRGBPalette16* getPaletteSelection(uint8_t i);

private:
    Menu* menu;
    CRGBArray<numLeds> leds;
    CRGBArray<numLeds> ledBackup; // For fade over from different effect
    unsigned long effectTime = 0;
    unsigned long lastUpdate = 0;
    EffectType currentEffect = EffectType::off;
    CRGB currentColor = CRGB(0, 0xA0, 0xFF);
    CRGB color2 = CRGB(0, 0x40, 0xFF);
    CRGBPalette16 currentPalette {CRGB::Black};
    unsigned long colorChangeTime = 0;
    uint8_t speed = 2;
    uint8_t brightnessLevel = 4;
    unsigned long menuTime = 0;
    uint8_t colorSelection = 0;
};

#endif
