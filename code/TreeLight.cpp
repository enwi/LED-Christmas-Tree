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
    if ((int)currentEffect + 1 >= (int)Effect::maxValue)
    {
        currentEffect = (Effect)0;
        effectTime = 0;
        colorChangeTime = 0;
    }
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
    if (effectTime > 60000) {
        // 30 seconds at normal speed
        effectTime = 0;
        updateColor();
    }
    switch (currentEffect)
    {
    case Effect::off:
        leds.fill_solid(CRGB::Black);
        break;
    case Effect::solid:
        leds.fill_solid(currentColor);
        break;
    case Effect::twoColorChange: {
        if (effectTime - colorChangeTime > 100)
        {
            std::swap(color2, currentColor);
            colorChangeTime += 100;
        }
        for (uint8_t i = 0; i < numLeds; ++i) {
            if ((i & 1) == 0) {
                leds[i] = currentColor;
            }
            else {
                leds[i] = color2;
            }
        }
        break;
    }
    case Effect::gradientHorizontal: {

        break;
    }
    case Effect::gradientVertical:
        break;
    }
}

void TreeLight::updateColor()
{
    currentColor = color2;
    color2.red = random(0, 256);
    color2.green = random(0, 256);
    color2.blue = random(0, 256);
}
