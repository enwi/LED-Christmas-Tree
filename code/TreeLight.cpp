#include "TreeLight.h"

// Effects
// two color change
// Color gradient bottom to top
// Gradient per ring
// Twinklefox
// solid color
// single color change
// rainbow
// running light

void TreeLight::init()
{
    FastLED.addLeds<APA106, pin, RGB>(leds, numLeds);
    FastLED.setBrightness(64);
    FastLED.setCorrection(LEDColorCorrection::Typical8mmPixel);
    leds.fill_solid(CRGB::Black);
    FastLED.show();
    lastUpdate = millis();
}

void TreeLight::nextEffect()
{
    currentEffect = (Effect)((int)currentEffect + 1);
    if (currentEffect >= Effect::maxValue)
    {
        currentEffect = (Effect)0;
    }
    effectTime = 0;
    colorChangeTime = 0;
}

void TreeLight::setEffect(Effect e)
{
    if (e != currentEffect && e < Effect::maxValue)
    {
        currentEffect = e;
        effectTime = 0;
        colorChangeTime = 0;
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
        return;
    }
    effectTime += (t - lastUpdate) * speed;
    lastUpdate = t;
    runEffect();
    FastLED.show();
}

void TreeLight::runEffect()
{
    const uint8_t rainbowDeltaHue = 32;
    bool doColorUpdate = false;
    switch (currentEffect)
    {
    case Effect::off:
        leds.fill_solid(CRGB::Black);
        break;
    case Effect::solid:
        leds.fill_solid(currentColor);
        doColorUpdate = true;
        break;
    case Effect::twoColorChange: {
        if (effectTime - colorChangeTime > 2000)
        {
            std::swap(color2, currentColor);
            colorChangeTime += 2000;
        }
        for (uint8_t i = 0; i < numLeds; ++i)
        {
            if ((i & 1) == 0)
            {
                leds[i] = currentColor;
            }
            else
            {
                leds[i] = color2;
            }
        }
        doColorUpdate = true;
        break;
    }
    case Effect::gradientHorizontal: {
        // Gradient between color and color2
        uint16_t blendVal = (uint16_t)(effectTime >> 5); // effectTime / 32 => full gradient in ~4s
        if (blendVal >= 512)
        {
            // Full gradient complete, transition to next color
            updateColor();
            blendVal = 0;
        }
        uint8_t blendStart = max((int)blendVal - 256, 0);
        uint8_t blendEnd = min((int)blendVal, 255);
        CRGB cStart = blend(currentColor, color2, blendStart);
        CRGB cEnd = blend(currentColor, color2, blendEnd);
        leds(0, 7).fill_gradient_RGB(cStart, cEnd);
        leds(8, 11).fill_gradient_RGB(cStart, cEnd);
        leds[12] = leds[0];
        break;
    }
    case Effect::gradientVertical: {
        // Gradient between color and color2
        uint16_t blendVal = (uint16_t)(effectTime >> 5); // effectTime / 32 => full gradient in ~4s
        if (blendVal >= 512)
        {
            // Full gradient complete, transition to next color
            updateColor();
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
        break;
    }
    case Effect::rainbowHorizontal: {
        uint8_t hue = (uint8_t)(effectTime >> 4); // effectTime / 16 => full rainbow in ~4s
        leds(0, 7).fill_rainbow(hue, rainbowDeltaHue);
        leds(8, 11).fill_rainbow(hue, rainbowDeltaHue * 2);
        leds[12] = leds[0];
        break;
    }
    case Effect::rainbowVertical: {
        CRGB colors[3];
        uint8_t hue = (uint8_t)(effectTime >> 4); // effectTime / 16 => full rainbow in ~4s
        fill_rainbow(colors, 3, hue, rainbowDeltaHue);
        // Bottom
        leds(0, 7).fill_solid(colors[0]);
        // Middle
        leds(8, 11).fill_solid(colors[1]);
        // Top
        leds[12] = colors[2];
        break;
    }
    case Effect::runningLight: {
        const uint8_t lightCount = 4; // max number of lit leds
        uint8_t nLights = (uint8_t)(effectTime >> 10); // effectTime / 1024 => about two leds per second
        leds.fill_solid(CRGB::Black);
        while (nLights > leds.size() + lightCount)
        {
            nLights -= leds.size() + lightCount;
        }
        if (nLights > 0)
        {
            uint8_t end = min(leds.size() - nLights + lightCount, 12);
            uint8_t start = max((int)leds.size() - nLights, 0);
            leds(start, end).fill_solid(currentColor);
        }
        else
        {
            // Only change color when currently empty
            doColorUpdate = true;
        }
        break;
    }
    }
    if (doColorUpdate && effectTime > 60000)
    {
        // 30 seconds at normal speed
        effectTime = 0;
        colorChangeTime = 0;
        updateColor();
    }
}

void TreeLight::updateColor()
{
    currentColor = color2;
    color2.red = random(0, 256);
    color2.green = random(0, 256);
    color2.blue = random(0, 256);
}
