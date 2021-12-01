#include "TreeLight.h"

// Effects
// two color change
// Color gradient bottom to top
// Gradient per ring
// Twinklefox
// solid color
// single color change

void TreeLight::init()
{
    FastLED.addLeds<NEOPIXEL, pin>(leds, numLeds);
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
    const uint8_t rainbowDeltaHue = 10;
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
        break;
    }
    case Effect::gradientVertical:
        break;
    case Effect::rainbowHorizontal: {
        uint8_t hue = (uint8_t)(effectTime >> 4); // effectTime / 16 => full rainbow in ~4s
        leds(0, 7).fill_rainbow(hue, rainbowDeltaHue);
        leds(8, 11).fill_rainbow(hue, rainbowDeltaHue*2);
        leds[12] = leds[0];
        break;
    }
    case Effect::rainbowVertical: {
        CRGB colors[3];
        uint8_t hue = (uint8_t)(effectTime >> 4); // effectTime / 16 => full rainbow in ~4s
        fill_rainbow(colors, 3, hue, rainbowDeltaHue * 2);
        // Bottom
        leds(0, 7).fill_solid(colors[0]);
        // Middle
        leds(8, 11).fill_solid(colors[1]);
        // Top
        leds[12] = colors[2];
        break;
    }
    }
    if (doColorUpdate && effectTime > 60000) {
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
