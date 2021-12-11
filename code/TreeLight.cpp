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

void TreeLight::init(Menu& menu)
{
    this->menu = &menu;
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

void TreeLight::nextEffect()
{
    currentEffect = (EffectType)((int)currentEffect + 1);
    if (currentEffect >= EffectType::maxValue)
    {
        currentEffect = (EffectType)0;
    }
    resetEffect();
}

void TreeLight::setEffect(EffectType e)
{
    if (e != currentEffect && e < EffectType::maxValue)
    {
        resetEffect();
    }
}

void TreeLight::nextSpeed()
{
    switch (speed)
    {
    case (uint8_t)Speed::stopped:
        speed = (uint8_t)Speed::slow;
        break;
    case (uint8_t)Speed::slow:
        speed = (uint8_t)Speed::medium;
        break;
    case (uint8_t)Speed::medium:
        speed = (uint8_t)Speed::fast;
        break;
    // case (uint8_t)Speed::fast:
    default:
        speed = (uint8_t)Speed::stopped;
        break;
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

void TreeLight::resetEffect()
{
    effectTime = 0;
    colorChangeTime = 0;
    // Save last effect colors
    ledBackup = leds;
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

void TreeLight::runEffect()
{
    const unsigned int colorDuration = 60000;
    const unsigned int startFadeIn = 2000;
    bool doColorUpdate = false;
    bool doFadeIn = true;

    switch (currentEffect)
    {
    case EffectType::off:
        leds.fill_solid(CRGB::Black);
        break;
    case EffectType::solid:
        leds.fill_solid(currentColor);
        doColorUpdate = true;
        break;
    case EffectType::twoColorChange: {
        effectTwoColorChange();
        doColorUpdate = true;
        break;
    }
    case EffectType::gradientHorizontal: {
        effectGradientHorizontal(doFadeIn);
        break;
    }
    case EffectType::gradientVertical: {
        effectGradientVertical(doFadeIn);
        break;
    }
    case EffectType::rainbowHorizontal: {
        effectRainbowHorizontal();
        break;
    }
    case EffectType::rainbowVertical: {
        effectRainbowVertical();
        break;
    }
    case EffectType::runningLight: {
        doColorUpdate = effectRunningLight();
        break;
    }
    case EffectType::twinkleFox: {
        effectTwinkleFox();
        doColorUpdate = true;
        break;
    }
    }
    if (speed > 0 && doFadeIn && effectTime < startFadeIn)
    {
        // TODO: does not work when speed = 0
        // Scale 0 to startFadeIn
        uint8_t fade = min((startFadeIn - effectTime) * 256 / startFadeIn, (unsigned long)255);
        fade = ease8InOutCubic(fade);
        leds.nblend(ledBackup, fade);
    }
    else if (doColorUpdate && effectTime > colorDuration)
    {
        // 30 seconds at normal speed
        resetEffect();
        updateColor();
    }
}

void TreeLight::effectGradientVertical(bool& doFadeIn)
{
    // Gradient between color and color2
    fract16 blendVal = (uint16_t)(effectTime >> 5); // effectTime / 32 => full gradient in ~4s
    if (blendVal >= 512)
    {
        // Full gradient complete, transition to next color
        updateColor();
        resetEffect();
        doFadeIn = false; // do not want to fade here, still same effect
        blendVal = 0;
    }
    uint8_t blendStart = max((int)blendVal - 256, 0);
    uint8_t blendEnd = min((int)blendVal, 255);
    CRGB cStart = blend(currentColor, color2, blendStart);
    CRGB cMiddle = blend(currentColor, color2, ((int)blendStart + blendEnd) / 2);
    CRGB cEnd = blend(currentColor, color2, blendEnd);
    leds(0, 7).fill_solid(cStart);
    leds(8, 11).fill_solid(cMiddle);
    leds[12] = cEnd;
}

void TreeLight::effectRainbowHorizontal()
{
    const uint8_t rainbowDeltaHue = 32;
    if (!isColorPalette())
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
            leds[i] = getPaletteColor(colorIndex);
            colorIndex += rainbowDeltaHue;
        }
        colorIndex = startIndex;
        for (uint8_t i = 8; i < 12; ++i)
        {
            leds[i] = getPaletteColor(colorIndex);
            colorIndex += rainbowDeltaHue * 2;
        }
        leds[12] = leds[0];
    }
}

void TreeLight::effectRainbowVertical()
{
    const uint8_t rainbowDeltaHue = 32;
    CRGB colors[3];
    uint8_t hue = (uint8_t)(effectTime >> 5); // effectTime / 32 => full rainbow in ~4s
    if (!isColorPalette())
    {
        fill_rainbow(colors, 3, hue, rainbowDeltaHue);
    }
    else
    {
        colors[0] = getPaletteColor(hue);
        colors[1] = getPaletteColor(hue + rainbowDeltaHue);
        colors[2] = getPaletteColor(hue + rainbowDeltaHue * 2);
    }
    // Bottom
    leds(0, 7).fill_solid(colors[0]);
    // Middle
    leds(8, 11).fill_solid(colors[1]);
    // Top
    leds[12] = colors[2];
}

void TreeLight::effectTwinkleFox()
{
    // From FastLED TwinkleFox example by Mark Kriegsman
    uint16_t prng16 = 11337;
    auto nextRandom = [](uint16_t prng) { return (uint16_t)((uint16_t)(prng * 2053) + 1384); };
    uint32_t clock32 = effectTime;
    CRGB bg = CRGB::Black; // Background color
    uint8_t backgroundBrightness = bg.getAverageLight();

    for (CRGB& pixel : leds)
    {
        prng16 = nextRandom(prng16);
        uint16_t clockOffset = prng16;
        prng16 = nextRandom(prng16);
        uint8_t speedMultiplier = ((((prng16 & 0xFF) >> 4) + (prng16 & 0x0F)) & 0x0F) + 0x08;
        uint32_t clock = (uint32_t)((clock32 * speedMultiplier) >> 3) + clockOffset;
        uint8_t uniqueSalt = prng16 >> 8;

        CRGB c = computeTwinkle(clock, uniqueSalt);

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
}

CRGB TreeLight::computeTwinkle(uint32_t clock, uint8_t salt)
{
    // From FastLED TwinkleFox example by Mark Kriegsman
    const uint8_t twinkleSpeed = 4; // 0-8
    const uint8_t twinkleDensity = 5; // 0-8
    const bool coolIncandescent = true; // fade out into red

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
        c = getPaletteColor(hue, false);
        c.nscale8_video(bright);
        if (coolIncandescent)
        {
            coolLikeIncandescent(c, fastCycle);
        }
    }
    return c;
}

void TreeLight::effectGradientHorizontal(bool& doFadeIn)
{
    // Gradient between color and color2
    fract16 blendVal = (uint16_t)(effectTime >> 5); // effectTime / 32 => full gradient in ~4s
    if (blendVal >= 512)
    {
        // Full gradient complete, transition to next color
        updateColor();
        resetEffect();
        doFadeIn = false; // do not want to fade in here, still same effect
        blendVal = 0;
    }
    uint8_t blendStart = max((int)blendVal - 256, 0);
    uint8_t blendEnd = min((int)blendVal, 255);
    CRGB cStart = blend(currentColor, color2, blendStart);
    CRGB cEnd = blend(currentColor, color2, blendEnd);
    leds(0, 7).fill_gradient_RGB(cStart, cEnd);
    leds(8, 11).fill_gradient_RGB(cStart, cEnd);
    leds[12] = leds[0];
}

void TreeLight::effectTwoColorChange()
{
    unsigned long dt = effectTime - colorChangeTime;
    CRGB c1 = currentColor;
    CRGB c2 = color2;
    if (dt >= 2000)
    {
        // swap
        color2 = c1;
        currentColor = c2;
        colorChangeTime += 2000;
        c1 = currentColor;
        c2 = color2;
    }
    else if (dt >= 2000 - 512)
    {
        // fade
        fract8 fade = (dt - (2000 - 512)) >> 1;
        fade = ease8InOutCubic(fade);
        c1 = blend(currentColor, color2, fade);
        c2 = blend(color2, currentColor, fade);
    }
    for (uint8_t i = 0; i < numLeds; ++i)
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
}

bool TreeLight::effectRunningLight()
{
    bool doColorUpdate = false;
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
            CRGB cEnd = currentColor;
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
            CRGB cStart = currentColor;
            cStart.nscale8_video(fadeIn);
            leds[start] = cStart;
        }
        // First led is out of bounds or set above
        ++start;

        if (end >= 0 && start < numLeds)
        {
            leds(start, end).fill_solid(currentColor);
        }
    }
    else
    {
        // Only change color when currently empty
        doColorUpdate = true;
    }
    return doColorUpdate;
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
            colorSelection = menu->getSubSelection();
            menuTime = t;
            updateColor();
            updateColor();
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
        case 3:
            color2 = ColorFromPalette(LavaColors_p, random(0, 255));
            break;
        case 4:
            color2 = ColorFromPalette(CloudColors_p, random(0, 255));
            break;
        case 5:
            color2 = ColorFromPalette(OceanColors_p, random(0, 255));
            break;
        case 6:
            color2 = ColorFromPalette(ForestColors_p, random(0, 255));
            break;
        default:
            color2.red = random(0, 255);
            color2.green = random(0, 255);
            color2.blue = random(0, 255);
            break;
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
    return colorSelection >= 3 && colorSelection < 7;
}

CRGB TreeLight::getPaletteColor(uint8_t mix, bool doBlend) const
{
    switch (colorSelection)
    {
    case 3:
        return ColorFromPalette(LavaColors_p, mix, 255, doBlend ? LINEARBLEND : NOBLEND);
    case 4:
        return ColorFromPalette(CloudColors_p, mix, 255, doBlend ? LINEARBLEND : NOBLEND);
    case 5:
        return ColorFromPalette(OceanColors_p, mix, 255, doBlend ? LINEARBLEND : NOBLEND);
    case 6:
        return ColorFromPalette(ForestColors_p, mix, 255, doBlend ? LINEARBLEND : NOBLEND);
    default:
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
        break;
    }
}
