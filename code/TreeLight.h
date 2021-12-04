#ifndef TREE_EFFECT_H
#define TREE_EFFECT_H
#include <Arduino.h>
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

enum class Effect
{
    off,
    solid,
    twoColorChange,
    gradientHorizontal,
    gradientVertical,
    rainbowHorizontal,
    rainbowVertical,
    runningLight,
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

    void nextEffect();
    void setEffect(Effect e);
    void nextSpeed();
    void setSpeed(Speed s);
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

    void setLED(const uint8_t start, const uint8_t end, const CRGB color)
    {
        if (start > leds.size() || end > leds.size())
        {
            return;
        }
        leds(start, end) = color;
        FastLED.show();
    }

public:
#ifdef ESP8266
    static constexpr uint8_t pin = D1;
#else
    static constexpr uint8_t pin = 3;
#endif
    static constexpr uint8_t numLeds = 13;

private:
    void runEffect();
    void displayMenu();
    void updateColor();

private:
    Menu* menu;
    CRGBArray<numLeds> leds;
    CRGBArray<numLeds> ledBackup; // For fade over from different effect
    unsigned long effectTime = 0;
    unsigned long lastUpdate = 0;
    Effect currentEffect = Effect::runningLight;
    CRGB currentColor = CRGB(0, 0xA0, 0xFF);
    CRGB color2 = CRGB(0, 0x40, 0xFF);
    unsigned long colorChangeTime = 0;
    uint8_t speed = 2;
    uint8_t brightnessLevel = 4;
};

#endif