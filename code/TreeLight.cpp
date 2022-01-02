#include "TreeLight.h"

namespace
{
    // A mostly red palette with green accents and white trim.
    // "CRGB::Gray" is used as white to keep the brightness more uniform.
    const TProgmemRGBPalette16 RedGreenWhite_p FL_PROGMEM
        = {CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
            CRGB::Gray, CRGB::Gray, CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green};

    // A mostly (dark) green palette with red berries.
    constexpr uint32_t Holly_Green = 0x00580c;
    constexpr uint32_t Holly_Red = 0xB00402;
    const TProgmemRGBPalette16 Holly_p FL_PROGMEM
        = {Holly_Green, Holly_Green, Holly_Green, Holly_Green, Holly_Green, Holly_Green, Holly_Green, Holly_Green,
            Holly_Green, Holly_Green, Holly_Green, Holly_Green, Holly_Green, Holly_Green, Holly_Green, Holly_Red};

    // A red and white striped palette
    // "CRGB::Gray" is used as white to keep the brightness more uniform.
    const TProgmemRGBPalette16 RedWhite_p FL_PROGMEM
        = {CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Red,
            CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray};

    // A mostly blue palette with white accents.
    // "CRGB::Gray" is used as white to keep the brightness more uniform.
    const TProgmemRGBPalette16 BlueWhite_p FL_PROGMEM
        = {CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
            CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Gray, CRGB::Gray, CRGB::Gray};

    // A pure "fairy light" palette with some brightness variations
    constexpr uint32_t HALFFAIRY = ((CRGB::FairyLight & 0xFEFEFE) / 2);
    constexpr uint32_t QUARTERFAIRY = ((CRGB::FairyLight & 0xFCFCFC) / 4);
    const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM = {CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight,
        CRGB::FairyLight, HALFFAIRY, HALFFAIRY, CRGB::FairyLight, CRGB::FairyLight, QUARTERFAIRY, QUARTERFAIRY,
        CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight};

    // A palette of soft snowflakes with the occasional bright one
    const TProgmemRGBPalette16 Snow_p FL_PROGMEM = {0x304048, 0x304048, 0x304048, 0x304048, 0x304048, 0x304048,
        0x304048, 0x304048, 0x304048, 0x304048, 0x304048, 0x304048, 0x304048, 0x304048, 0x304048, 0xE0F0FF};

    // A palette reminiscent of large 'old-school' C9-size tree lights
    // in the five classic colors: red, orange, green, blue, and white.
    constexpr uint32_t C9_Red = 0xB80400;
    constexpr uint32_t C9_Orange = 0x902C02;
    constexpr uint32_t C9_Green = 0x046002;
    constexpr uint32_t C9_Blue = 0x070758;
    constexpr uint32_t C9_White = 0x606820;
    const TProgmemRGBPalette16 RetroC9_p FL_PROGMEM = {C9_Red, C9_Orange, C9_Red, C9_Orange, C9_Orange, C9_Red,
        C9_Orange, C9_Red, C9_Green, C9_Green, C9_Green, C9_Green, C9_Blue, C9_Blue, C9_Blue, C9_White};

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

const TProgmemRGBPalette16* TreeLight::getPaletteSelection(uint8_t i)
{
    static const TProgmemRGBPalette16* palettes[] = {nullptr, // harmonic colors
        nullptr, //
        nullptr, //
        &Holly_p, //
        &RedGreenWhite_p, //
        &RetroC9_p, //
        &FairyLight_p, //
        &BlueWhite_p};

    if (i < (sizeof(palettes) / sizeof(palettes[0])))
    {
        return palettes[i];
    }
    return nullptr;
}

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
    EffectControl runEffect(TreeLightView& lights, CRGBSet& leds, unsigned long effectTime) override
    {
        leds.fill_solid(CRGB::Black);
        return {};
    }

    const char* getName() const override { return "off"; }
};

class SolidEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, CRGBSet& leds, unsigned long effectTime) override
    {
        leds.fill_solid(lights.firstColor());
        EffectControl result;
        result.allowAutoColorChange = true;
        return result;
    }

    const char* getName() const override { return "solid"; }
};

class HorizontalRainbowEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, CRGBSet& leds, unsigned long effectTime) override
    {
        if (!lights.isColorPalette())
        {
            uint8_t hue = (uint8_t)(effectTime >> 5); // effectTime / 32 => full rainbow in ~4s
            leds(0, 7).fill_rainbow(hue, rainbowDeltaHue);
            leds(8, 11).fill_rainbow(hue, rainbowDeltaHue * 2);
            leds[12] = leds[0];
        }
        else
        {
            uint8_t startIndex = (uint8_t)(effectTime >> 5); // effectTime / 32 => full rainbow in ~4s
            uint8_t colorIndex = startIndex;
            for (uint8_t i = 0; i < 8; ++i)
            {
                leds[i] = lights.getPaletteColor(colorIndex);
                colorIndex += rainbowDeltaHue;
            }
            colorIndex = startIndex;
            for (uint8_t i = 8; i < 12; ++i)
            {
                leds[i] = lights.getPaletteColor(colorIndex);
                colorIndex += rainbowDeltaHue * 2;
            }
            leds[12] = leds[0];
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
    EffectControl runEffect(TreeLightView& lights, CRGBSet& leds, unsigned long effectTime) override
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
        leds(0, 7).fill_solid(colors[0]);
        // Middle
        leds(8, 11).fill_solid(colors[1]);
        // Top
        leds[12] = colors[2];

        return {};
    }

    const char* getName() const override { return "rainbowVertical"; }

private:
    uint8_t rainbowDeltaHue = 32;
};

class HorizontalGradientEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, CRGBSet& leds, unsigned long effectTime) override
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
        leds(0, 7).fill_gradient_RGB(cStart, cEnd);
        leds(8, 11).fill_gradient_RGB(cStart, cEnd);
        leds[12] = leds[0];

        return result;
    }

    const char* getName() const override { return "gradientHorizontal"; }
};

class VerticalGradientEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, CRGBSet& leds, unsigned long effectTime) override
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
        leds(0, 7).fill_solid(cStart);
        leds(8, 11).fill_solid(cMiddle);
        leds[12] = cEnd;

        return result;
    }

    const char* getName() const override { return "gradientVertical"; }
};

class TwinkleFoxEffect : public IEffect
{
public:
    EffectControl runEffect(TreeLightView& lights, CRGBSet& leds, unsigned long effectTime) override
    {
        // From FastLED TwinkleFox example by Mark Kriegsman
        uint16_t prng16 = 11337;
        auto nextRandom = [](uint16_t prng) { return (uint16_t)((uint16_t)(prng * 2053) + 1384); };
        uint32_t clock32 = effectTime;
        uint8_t backgroundBrightness = bg.getAverageLight();

        for (CRGB& pixel : leds)
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
                pixel = c;
            }
            else if (deltaBright > 0)
            {
                // Slightly brighter than background, blend
                pixel = blend(bg, c, deltaBright * 8);
            }
            else
            {
                // Pixel is not brighter than background
                pixel = bg;
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
    EffectControl runEffect(TreeLightView& lights, CRGBSet& leds, unsigned long effectTime) override
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
        for (uint8_t i = 0; i < leds.size(); ++i)
        {
            if ((i & 1) == 0)
            {
                leds[i] = c1;
            }
            else
            {
                leds[i] = c2;
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
    EffectControl runEffect(TreeLightView& lights, CRGBSet& leds, unsigned long effectTime) override
    {
        EffectControl result;
        uint8_t numLeds = leds.size();
        const uint8_t lightCount = 4; // max number of lit leds
        uint8_t nLights = (uint8_t)(effectTime - colorChangeTime >> 9); // effectTime / 512 => about 4 leds per second
        uint16_t fade = (uint16_t)(effectTime - colorChangeTime >> 0) & 0x1FF;
        leds.fill_solid(CRGB::Black);
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
                leds[end] = cEnd;
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
                leds[start] = cStart;
            }
            // First led is out of bounds or set above
            ++start;

            if (end >= 0 && start < numLeds)
            {
                leds(start, end).fill_solid(lights.firstColor());
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

    EffectControl runEffect(TreeLightView& lights, CRGBSet& leds, unsigned long effectTime) override
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

void TreeLight::init(Menu& menu)
{
    this->menu = &menu;
    effectList = createEffects();
    currentEffect = effectList[0];
    currentEffect->reset(false);
#if defined(ESP32) || defined(ESP8266)
    FastLED.addLeds<APA106, pin, RGB>(leds, numLeds);
#else
    FastLED.addLeds<WS2812, pin, GRB>(leds, numLeds);
#endif

    setBrightnessLevel(4);
    FastLED.setCorrection(LEDColorCorrection::Typical8mmPixel);
    leds.fill_solid(CRGB::Black);
    FastLED.show();
    lastUpdate = millis();
    ledBackup.fill_solid(CRGB::Black);
    setColorSelection(0);

    // Init random seed
#if defined(ESP32) || defined(ESP8266)
    randomSeed(ESP.getVcc() * analogRead(A0));
#else
    randomSeed(analogRead(A0) * 17 + 23);
#endif
    // Initialize random colors
    updateColor();
    updateColor();
}

void TreeLight::getStatusJsonString(JsonObject& output)
{
    auto&& lights = output.createNestedObject("lights");
    lights["brightness"] = getBrightnessLevel();
    lights["speed"] = getSpeed();
    lights["effect"] = (int)getEffectType();
    JsonArray effects = lights.createNestedArray("effects");
    for (size_t i = 0; i < (size_t)EffectType::maxValue; ++i)
    {
        effects.add(effectList[i]->getName());
    }
    lights["color"] = colorSelection;
    // TODO remove hard code
    JsonArray colors = lights.createNestedArray("colors");
    colors.add("Random1");
    colors.add("Random2");
    colors.add("Random3");
    colors.add("Holly");
    colors.add("RGW");
    colors.add("RetroC9");
    colors.add("FairyLight");
    colors.add("BW");
}

void TreeLight::nextEffect()
{
    currentEffectType = (EffectType)((int)currentEffectType + 1);
    if (currentEffectType >= EffectType::maxValue)
    {
        currentEffectType = (EffectType)0;
    }
    currentEffect = effectList[(int)currentEffectType];
    resetEffect(false);
}

void TreeLight::setEffect(EffectType e)
{
    if (e != currentEffectType && e < EffectType::maxValue)
    {
        currentEffectType = e;
        currentEffect = effectList[(int)e];
        resetEffect(false);
    }
}

void TreeLight::nextSpeed()
{
    // Web interface speed may be different from these stages
    if (speed <= (uint8_t)Speed::stopped)
    {
        speed = (uint8_t)Speed::slow;
    }
    else if (speed <= (uint8_t)Speed::slow)
    {
        speed = (uint8_t)Speed::medium;
    }
    else if (speed <= (uint8_t)Speed::medium)
    {
        speed = (uint8_t)Speed::fast;
    }
    else
    {
        speed = (uint8_t)Speed::stopped;
    }
}

void TreeLight::setSpeed(Speed s)
{
    if ((uint8_t)s != speed && s < Speed::maxValue)
    {
        speed = (uint8_t)s;
    }
}

void TreeLight::update()
{
    unsigned long t = millis();
    if (t - lastUpdate < 10)
    {
        FastLED.show();
        return;
    }
    if (menu->isActive())
    {
        displayMenu();
    }
    else
    {
        effectTime += (t - lastUpdate) * speed;
        runEffect();
    }
    lastUpdate = t;
    FastLED.show();
}

void TreeLight::resetEffect(bool timerOnly)
{
    effectTime = 0;
    // Save last effect colors
    ledBackup = leds;
    if (currentEffect)
    {
        currentEffect->reset(timerOnly);
    }
}

void TreeLight::setBrightnessLevel(uint8_t level)
{
    brightnessLevel = level;
    uint8_t scale = 0;
    switch (level)
    {
    case 1:
        scale = 16;
        break;
    case 2:
        scale = 32;
        break;
    case 3:
        scale = 48;
        break;
    case 4:
        scale = 64;
        break;
    case 5:
        scale = 96;
        break;
    case 6:
        scale = 128;
        break;
    case 7:
        scale = 192;
        break;
    case 8:
        scale = 255;
        break;
    }
    FastLED.setBrightness(scale);
}

void TreeLight::initColorMenu()
{
    menuTime = millis();
    menu->setMenuState(Menu::MenuState::colorSelect);
    menu->setNumSubSelections(8);
    menu->setSubSelection(colorSelection);
}

void TreeLight::setColorSelection(uint8_t index)
{
    if (index != colorSelection)
    {
        const TProgmemRGBPalette16* palette = getPaletteSelection(index);
        if (palette != nullptr)
        {
            currentPalette = *palette;
        }
        else
        {
            fill_solid(currentPalette, 16, CRGB::Black);
        }
        colorSelection = index;
        updateColor();
        updateColor();
    }
}

void TreeLight::runEffect()
{
    const unsigned int colorDuration = 60000;
    const unsigned int startFadeIn = 2000;
    IEffect::EffectControl c;

    TreeLightView v(*this);
    if (currentEffect != nullptr)
    {
        c = currentEffect->runEffect(v, leds, effectTime);
    }

    if (speed > 0 && c.fadeOver && effectTime < startFadeIn)
    {
        // TODO: does not work when speed = 0
        // Scale 0 to startFadeIn
        uint8_t fade = min((startFadeIn - effectTime) * 256 / startFadeIn, (unsigned long)255);
        fade = ease8InOutCubic(fade);
        leds.nblend(ledBackup, fade);
    }
    else if (c.allowAutoColorChange && effectTime > colorDuration)
    {
        // 30 seconds at normal speed
        resetEffect();
        updateColor();
    }
}

void TreeLight::displayMenu()
{
    leds.fill_solid(CRGB::Black);
    CRGB color = CRGB::White;
    if (menu->getMenuState() == Menu::MenuState::mainSelect)
    {
        switch (menu->getLongPressMode())
        {
        case 1:
            leds[12] = color;
            break;
        case 2:
            leds(8, 11) = color;
            break;
        case 3:
            leds(0, 7) = color;
            break;
        }
    }
    else if (menu->getMenuState() == Menu::MenuState::brightnessSelect)
    {
        leds(0, brightnessLevel - 1) = color;
    }
    else if (menu->getMenuState() == Menu::MenuState::colorSelect)
    {
        unsigned long t = millis();
        // Show color
        if (colorSelection != menu->getSubSelection())
        {
            setColorSelection(menu->getSubSelection());
            menuTime = t;
        }
        else if (t - menuTime > 1000)
        {
            menuTime = t;
            updateColor();
        }
        leds[12] = currentColor;
        if (isColorPalette())
        {
            uint8_t mix = (t / 64);
            for (uint8_t i = 8; i < 12; ++i)
            {
                leds[i] = getPaletteColor(mix, false);
                mix += 64;
            }
        }
        else
        {
            leds(8, 11) = color2;
        }
        leds[colorSelection] = CRGB::White;
    }
}

///@brief Generate a harmonic color to the given @ref color
///
/// Adapted from http://devmag.org.za/2012/07/29/how-to-choose-colours-procedurally-algorithms/
///
/// Analogous: Choose second and third ranges 0.
/// Complementary: Choose the third range 0, and first offset angle 180.
/// Split Complementary: Choose offset angles 180 +/- a small angle.
/// The second and third ranges must be smaller than the difference between the two offset angles.
/// Triad: Choose offset angles 120 and 240.
///
///@param color Color to generate harmonic color to
///@param offsetAngle1 [0-255]
///@param offsetAngle2 [0-255]
///@param rangeAngle0 [0-255]
///@param rangeAngle1 [0-255]
///@param rangeAngle2 [0-255]
///@param saturation Saturation of the generated color [0-255]
///@param luminance Luminance of the generated color [0-255]
///@return CRGB
static CRGB GenerateHarmonicColor(const CHSV color, uint8_t offsetAngle1, uint8_t offsetAngle2, uint8_t rangeAngle0,
    uint8_t rangeAngle1, uint8_t rangeAngle2, uint8_t saturation, uint8_t luminance)
{
    // const uint8_t referenceAngle = random(0, 256);
    const uint8_t referenceAngle = color.h;
    uint8_t randomAngle = random(1, 255) * (rangeAngle0 + rangeAngle1 + rangeAngle2) / 256;

    if (randomAngle > rangeAngle0)
    {
        if (randomAngle < rangeAngle0 + rangeAngle1)
        {
            randomAngle += offsetAngle1;
        }
        else
        {
            randomAngle += offsetAngle2;
        }
    }

    return CHSV(referenceAngle + randomAngle, saturation, luminance);
}

void TreeLight::updateColor()
{
    currentColor = color2;

    CRGB colorDifference;
    for (uint8_t i = 0; i < 4; ++i)
    {
        if (isColorPalette())
        {
            color2 = ColorFromPalette(currentPalette, random(0, 255));
        }
        else
        {
            switch (colorSelection)
            {
            case 0:
                color2 = GenerateHarmonicColor(rgb2hsv_approximate(color2), 16, 32, 8, 16, 32, 255, 255);
                break;
            case 1:
                color2 = GenerateHarmonicColor(rgb2hsv_approximate(color2), 16, 32, 8, 16, 32, 128, 255);
                break;
            case 2:
                color2 = GenerateHarmonicColor(CHSV(0, 255, 255), 16, 32, 8, 0, 0, 255, 255);
                break;
            default:
                color2 = CRGB::Black;
            }
        }
        colorDifference = currentColor;
        colorDifference -= color2;
        if (colorDifference.getAverageLight() > 8)
        {
            return;
        }
    }
}

bool TreeLight::isColorPalette() const
{
    return getPaletteSelection(colorSelection) != nullptr;
}

CRGB TreeLight::getPaletteColor(uint8_t mix, bool doBlend) const
{
    if (isColorPalette())
    {
        return ColorFromPalette(currentPalette, mix, 255, doBlend ? LINEARBLEND : NOBLEND);
    }
    else
    {
        if (!doBlend)
        {
            if (mix < 128)
            {
                return currentColor;
            }
            else
            {
                return color2;
            }
        }
        else
        {
            return blend(currentColor, color2, mix);
        }
    }
}
