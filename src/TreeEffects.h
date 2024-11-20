#ifndef TREE_EFFECTS_H
#define TREE_EFFECTS_H

#include <FastLEDCompat.h>

// These effect types have to match the order in createEffects() in the cpp file
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
    cycling,
    maxValue // Not an effect, number of valid effects
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

    virtual void reset(bool timerOnly) {}; // timerOnly is true when effectTime was reset, false for a full effect reset
    virtual EffectControl runEffect(class TreeLightView& lights, PixelBus& leds, unsigned long effectTime) = 0;
    virtual const char* getName() const = 0;
};

// Returns array with EffectType::maxValue elements
IEffect** createEffects();

#endif
