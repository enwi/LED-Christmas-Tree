#include "TreeEffects.h"

#include <FastLED.h>
#include <stdint.h>

#include "TreeLight.h"

namespace
{
    uint8_t attackDecayWave8(uint8_t i)
    {
        // See FastLED TwinkleFox example
        if (i < 86)
        {
            return i * 3;
        }
        else
        {
            i -= 86;
            return 255 - (i + (i / 2));
        }
    }
    void coolLikeIncandescent(CRGB& c, uint8_t phase)
    {
        // See FastLED TwinkleFox example
        if (phase < 128)
            return;
        uint8_t cooling = (phase - 128) >> 4;
        c.g = qsub8(c.g, cooling);
        c.b = qsub8(c.b, cooling * 2);
    }
} // namespace

// Effects
// two color change
// Color gradient bottom to top
// Gradient per ring
// Twinklefox
// solid color
// single color change
// rainbow
// running light

class OffEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, PixelBus& leds, unsigned long effectTime) override
    {
        leds.ClearTo(CRGB::Black);
        return {};
    }

    const char* getName() const override { return "off"; }
};

class SolidEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, PixelBus& leds, unsigned long effectTime) override
    {
        leds.ClearTo(toRgb(lights.firstColor()));
        EffectControl result;
        result.allowAutoColorChange = true;
        return result;
    }

    const char* getName() const override { return "solid"; }
};

class HorizontalRainbowEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, PixelBus& leds, unsigned long effectTime) override
    {
        if (!lights.isColorPalette())
        {
            uint8_t hue = (uint8_t)(effectTime >> 5); // effectTime / 32 => full rainbow in ~4s
            fillRainbow(leds, 0, 7, hue, rainbowDeltaHue);
            fillRainbow(leds, 8, 11, hue, rainbowDeltaHue * 2);
            leds.SetPixelColor(12, toRgb(CHSV(hue, 240, 255)));
        }
        else
        {
            uint8_t startIndex = (uint8_t)(effectTime >> 5); // effectTime / 32 => full rainbow in ~4s
            uint8_t colorIndex = startIndex;
            for (uint8_t i = 0; i < 8; ++i)
            {
                leds.SetPixelColor(i, toRgb(lights.getPaletteColor(colorIndex)));
                colorIndex += rainbowDeltaHue;
            }
            colorIndex = startIndex;
            for (uint8_t i = 8; i < 12; ++i)
            {
                leds.SetPixelColor(i, toRgb(lights.getPaletteColor(colorIndex)));
                colorIndex += rainbowDeltaHue * 2;
            }
            leds.SetPixelColor(12, toRgb(lights.getPaletteColor(startIndex)));
        }
        return {};
    }

    const char* getName() const override { return "rainbowHorizontal"; }

private:
    uint8_t rainbowDeltaHue = 32;
};

class VerticalRainbowEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, PixelBus& leds, unsigned long effectTime) override
    {
        CRGB colors[3];
        uint8_t hue = (uint8_t)(effectTime >> 5); // effectTime / 32 => full rainbow in ~4s
        if (!lights.isColorPalette())
        {
            fill_rainbow(colors, 3, hue, rainbowDeltaHue);
        }
        else
        {
            colors[0] = lights.getPaletteColor(hue);
            colors[1] = lights.getPaletteColor(hue + rainbowDeltaHue);
            colors[2] = lights.getPaletteColor(hue + rainbowDeltaHue * 2);
        }
        // Bottom
        leds.ClearTo(toRgb(colors[0]), 0, 7);
        // Middle
        leds.ClearTo(toRgb(colors[1]), 8, 11);
        // Top
        leds.SetPixelColor(12, toRgb(colors[2]));

        return {};
    }

    const char* getName() const override { return "rainbowVertical"; }

private:
    uint8_t rainbowDeltaHue = 32;
};

class HorizontalGradientEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, PixelBus& leds, unsigned long effectTime) override
    {
        EffectControl result;
        fract16 blendVal = (uint16_t)(effectTime >> 5); // effectTime / 32 => full gradient in ~4s
        if (blendVal >= 512)
        {
            // Full gradient complete, transition to next color
            lights.updateColor();
            lights.resetEffect();
            result.fadeOver = false; // do not want to fade here, still same effect
            blendVal = 0;
        }
        uint8_t blendStart = max((int)blendVal - 256, 0);
        uint8_t blendEnd = min((int)blendVal, 255);
        CRGB cStart = blend(lights.firstColor(), lights.secondColor(), blendStart);
        CRGB cMiddle = blend(lights.firstColor(), lights.secondColor(), ((int)blendStart + blendEnd) / 2);
        CRGB cEnd = blend(lights.firstColor(), lights.secondColor(), blendEnd);
        fillGradientRgb(leds, 0, 7, cStart, cEnd);
        fillGradientRgb(leds, 8, 11, cStart, cEnd);
        leds.SetPixelColor(12, toRgb(cStart));

        return result;
    }

    const char* getName() const override { return "gradientHorizontal"; }
};

class VerticalGradientEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, PixelBus& leds, unsigned long effectTime) override
    {
        EffectControl result;
        fract16 blendVal = (uint16_t)(effectTime >> 5); // effectTime / 32 => full gradient in ~4s
        if (blendVal >= 512)
        {
            // Full gradient complete, transition to next color
            lights.updateColor();
            lights.resetEffect();
            result.fadeOver = false; // do not want to fade here, still same effect
            blendVal = 0;
        }
        uint8_t blendStart = max((int)blendVal - 256, 0);
        uint8_t blendEnd = min((int)blendVal, 255);
        CRGB cStart = blend(lights.firstColor(), lights.secondColor(), blendStart);
        CRGB cMiddle = blend(lights.firstColor(), lights.secondColor(), ((int)blendStart + blendEnd) / 2);
        CRGB cEnd = blend(lights.firstColor(), lights.secondColor(), blendEnd);
        leds.ClearTo(toRgb(cStart), 0, 7);
        leds.ClearTo(toRgb(cMiddle), 8, 11);
        leds.SetPixelColor(12, toRgb(cEnd));

        return result;
    }

    const char* getName() const override { return "gradientVertical"; }
};

class TwinkleFoxEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, PixelBus& leds, unsigned long effectTime) override
    {
        // From FastLED TwinkleFox example by Mark Kriegsman
        uint16_t prng16 = 11337;
        auto nextRandom = [](uint16_t prng) { return (uint16_t)((uint16_t)(prng * 2053) + 1384); };
        uint32_t clock32 = effectTime;
        uint8_t backgroundBrightness = bg.getAverageLight();

        for (uint8_t i = 0; i < leds.PixelCount(); ++i)
        {

            prng16 = nextRandom(prng16);
            uint16_t clockOffset = prng16;
            prng16 = nextRandom(prng16);
            uint8_t speedMultiplier = ((((prng16 & 0xFF) >> 4) + (prng16 & 0x0F)) & 0x0F) + 0x08;
            uint32_t clock = (uint32_t)((clock32 * speedMultiplier) >> 3) + clockOffset;
            uint8_t uniqueSalt = prng16 >> 8;

            CRGB c = computeTwinkle(lights, clock, uniqueSalt);

            uint8_t cBright = c.getAverageLight();
            int16_t deltaBright = cBright - backgroundBrightness;
            if (deltaBright >= 32 || (!bg))
            {
                // New pixel is significantly brighter than background
                leds.SetPixelColor(i, toRgb(c));
            }
            else if (deltaBright > 0)
            {
                // Slightly brighter than background, blend
                leds.SetPixelColor(i, toRgb(blend(bg, c, deltaBright * 8)));
            }
            else
            {
                // Pixel is not brighter than background
                leds.SetPixelColor(i, toRgb(bg));
            }
        }
        EffectControl result;
        result.allowAutoColorChange = true;
        return result;
    }
    CRGB computeTwinkle(TreeLightView& lights, uint32_t clock, uint8_t salt)
    {
        // From FastLED TwinkleFox example by Mark Kriegsman

        uint16_t ticks = clock >> (8 - twinkleSpeed);
        uint8_t fastCycle = ticks;
        uint16_t slowCycle = (ticks >> 8) + salt;
        slowCycle += sin8(slowCycle);
        slowCycle = (slowCycle * 2053) + 1384;
        uint8_t slowCycle8 = (slowCycle & 0xFF) + (slowCycle >> 8);

        uint8_t bright = 0;
        if ((slowCycle8 & 0x0E) / 2 < twinkleDensity)
        {
            bright = attackDecayWave8(fastCycle);
        }

        uint8_t hue = slowCycle8 - salt;
        CRGB c = CRGB::Black;
        if (bright > 0)
        {
            c = lights.getPaletteColor(hue, false);
            c.nscale8_video(bright);
            if (coolIncandescent)
            {
                coolLikeIncandescent(c, fastCycle);
            }
        }
        return c;
    }

    const char* getName() const override { return "twinkleFox"; }

private:
    uint8_t twinkleSpeed = 4; // 0-8
    uint8_t twinkleDensity = 5; // 0-8
    bool coolIncandescent = true; // fade out into red
    CRGB bg = CRGB::Black; // Background color
};

class TwoColorChangeEffect : public IEffect
{
public:
    void reset(bool timerOnly)
    {
        colorChangeTime = 0;
        swapped = false;
    }
    EffectControl runEffect(TreeLightView& lights, PixelBus& leds, unsigned long effectTime) override
    {
        unsigned long dt = effectTime - colorChangeTime;
        uint16_t fadeTime = 1 << (fadeDuration + 8);
        CRGB first = getColor(lights, false);
        CRGB second = getColor(lights, true);
        CRGB c1;
        CRGB c2;
        if (dt >= swapTime)
        {
            // swap
            swapped = !swapped;
            colorChangeTime += swapTime;

            c1 = second;
            c2 = first;
        }
        else if (dt >= swapTime - fadeTime)
        {
            // fade
            fract8 fade = (dt - (swapTime - fadeTime)) >> 1;
            fade = ease8InOutCubic(fade);
            c1 = blend(first, second, fade);
            c2 = blend(second, first, fade);
        }
        else
        {
            c1 = first;
            c2 = second;
        }
        for (uint8_t i = 0; i < leds.PixelCount(); ++i)
        {
            if ((i & 1) == 0)
            {
                leds.SetPixelColor(i, toRgb(c1));
            }
            else
            {
                leds.SetPixelColor(i, toRgb(c2));
            }
        }
        EffectControl result;
        result.allowAutoColorChange = true;
        return result;
    }

    CRGB getColor(TreeLightView& lights, bool second)
    {
        return (swapped == second) ? lights.firstColor() : lights.secondColor();
    }

    const char* getName() const override { return "twoColorChange"; }

private:
    unsigned long colorChangeTime = 0;
    uint16_t swapTime = 2000;
    uint8_t fadeDuration = 1; // 1<<(fadeSpeed+8) must be less than swapTime
    bool swapped = false;
};

class RunningLightEffect : public IEffect
{
public:
    void reset(bool timerOnly) { colorChangeTime = 0; }
    EffectControl runEffect(TreeLightView& lights, PixelBus& leds, unsigned long effectTime) override
    {
        EffectControl result;
        uint8_t numLeds = leds.PixelCount();
        const uint8_t lightCount = 4; // max number of lit leds
        uint8_t nLights = (uint8_t)((effectTime - colorChangeTime) >> 9); // effectTime / 512 => about 4 leds per second
        uint16_t fade = (uint16_t)((effectTime - colorChangeTime) >> 0) & 0x1FF;
        leds.ClearTo(toRgb(CRGB::Black));
        if (nLights >= numLeds + lightCount)
        {
            nLights = 0;
            colorChangeTime += (unsigned long)(numLeds + lightCount) << 9;
        }
        if (nLights > 0)
        {
            int end = min((int)numLeds - nLights + lightCount - 1, (int)numLeds);

            if (end != numLeds)
            {
                // Last led can fade out
                // 256 <= fade < 512: fade out end
                fract8 fadeOut = 255 - max((int)fade - 256, 0);
                fadeOut = ease8InOutCubic(fadeOut);
                CRGB cEnd = lights.firstColor();
                cEnd.nscale8_video(fadeOut);
                leds.SetPixelColor(end, toRgb(cEnd));
            }
            // Last led is out of bounds or set above
            --end;

            int start = max((int)numLeds - nLights, -1);
            if (start != -1)
            {
                // First led can fade in
                // 0 <= fade < 256: fade in start
                fract8 fadeIn = min(fade, (uint16_t)255);
                fadeIn = ease8InOutCubic(fadeIn);
                CRGB cStart = lights.firstColor();
                cStart.nscale8_video(fadeIn);
                leds.SetPixelColor(start, toRgb(cStart));
            }
            // First led is out of bounds or set above
            ++start;

            if (end >= 0 && start < numLeds)
            {
                leds.ClearTo(toRgb(lights.firstColor()), start, end);
            }
        }
        else
        {
            // Only change color when currently empty
            result.allowAutoColorChange = true;
        }
        return result;
    }
    const char* getName() const override { return "runningLight"; }

private:
    unsigned long colorChangeTime = 0;
};

template <uint8_t N>
class CyclingEffect : public IEffect
{
public:
    CyclingEffect<N>()
    {
        for (uint8_t i = 0; i < N; ++i)
        {
            effectList[i] = nullptr;
            effectCycles[i] = 1;
        }
    }

    void setEffect(uint8_t idx, IEffect* e)
    {
        if (idx < N)
        {
            effectList[idx] = e;
        }
    }
    void setEffects(IEffect* const* effects, uint8_t num, uint8_t start = 0)
    {
        uint8_t max = start + num;
        if (max > N)
        {
            max = N;
        }
        for (uint8_t i = start; i < max; ++i)
        {
            effectList[i] = *effects;
            ++effects;
        }
    }
    void setEffectCycles(uint8_t idx, uint8_t numCycles)
    {
        if (idx < N)
        {
            effectCycles[idx] = numCycles;
        }
    }

    void reset(bool timerOnly) override
    {
        if (timerOnly)
        {
            // this is called after every color change, so do not reset but update the effect
            ++effectColors;
            if (effectColors >= effectCycles[effectIdx])
            {
                nextEffect();
            }
            else
            {
                IEffect* e = effectList[effectIdx];
                if (e)
                {
                    e->reset(true);
                }
            }
        }
        else
        {
            effectIdx = 0;
            effectColors = 0;
            IEffect* e = effectList[effectIdx];
            if (e)
            {
                e->reset(false);
            }
        }
    }
    void nextEffect()
    {
        effectColors = 0;
        ++effectIdx;
        if (effectIdx >= N)
        {
            effectIdx = 0;
        }
        IEffect* e = effectList[effectIdx];
        if (e)
        {
            e->reset(false);
        }
    }

    EffectControl runEffect(TreeLightView& lights, PixelBus& leds, unsigned long effectTime) override
    {
        IEffect* e = effectList[effectIdx];
        EffectControl result;
        if (e)
        {
            result = e->runEffect(lights, leds, effectTime);
            if (effectTime >= maxEffectTime)
            {
                // Effects that do not cycle by themselves
                lights.resetEffect(true);
            }
        }
        else
        {
            nextEffect();
        }
        return result;
    }

    const char* getName() const override { return "cycling"; }

private:
    IEffect* effectList[N];
    unsigned long maxEffectTime = 60000;
    uint8_t effectCycles[N];
    uint8_t effectColors = 0;
    uint8_t effectIdx = 0;
};

IEffect** createEffects()
{
    static OffEffect off;
    static SolidEffect solid;
    static TwoColorChangeEffect twoColor;
    static HorizontalGradientEffect gradientHorizontal;
    static VerticalGradientEffect gradientVertical;
    static HorizontalRainbowEffect rainbowHorizontal;
    static VerticalRainbowEffect rainbowVertical;
    static RunningLightEffect runningLight;
    static TwinkleFoxEffect twinkleFox;
    static CyclingEffect<(uint8_t)EffectType::maxValue - 2> cycling;
    static IEffect* e[(int)EffectType::maxValue] = {&off, &solid, &twoColor, &gradientHorizontal, &gradientVertical,
        &rainbowHorizontal, &rainbowVertical, &runningLight, &twinkleFox, &cycling};
    cycling.setEffects(e + 1, (uint8_t)EffectType::maxValue - 2);
    cycling.setEffectCycles((uint8_t)EffectType::gradientHorizontal - 1, 4);
    cycling.setEffectCycles((uint8_t)EffectType::gradientVertical - 1, 4);
    return e;
}
