#ifndef TREE_EFFECT_H
#define TREE_EFFECT_H
#include <Arduino.h>
#include <FastLED.h>

// Config:
// - brighness
// - color (rgb/palette)
// - effect
// - on/off
// - speed (off,slow,normal,fast)

enum class Effect
{
    off,
    solid,
    twoColorChange,
    gradientHorizontal,
    gradientVertical,
    maxValue // Not an effect, number of valid effects
};

class TreeLight
{
public:
    void init();

    void nextEffect();
    void setEffect(Effect e);
    void update();

public:
    static constexpr uint8_t pin = D1;
    static constexpr uint8_t numLeds = 13;
private:
    void runEffect();
    void updateColor();
private:
    CRGBArray<numLeds> leds;
    unsigned long effectTime = 0;
    unsigned long lastUpdate = 0;
    Effect currentEffect = Effect::off;
    CRGB currentColor = CRGB(0, 0xA0, 0xFF);
    CRGB color2 = CRGB(0, 0x40, 0xFF);
    unsigned long colorChangeTime = 0;
    uint8_t speed = 2;
};

#endif