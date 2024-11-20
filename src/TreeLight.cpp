#include "TreeLight.h"

#include <Constants.h>

void TreeLight::init(Menu& menu)
{
    this->menu = &menu;
    effectList = createEffects();
    currentEffect = effectList[0];
    currentEffect->reset(false);
#if defined(ESP8266)
    // Uses RX pin instead of GPIO pin
    pinMode(pin, INPUT);
#endif
    leds.reset(new PixelBus(numLeds, pin));
    leds->Begin();

    setBrightnessLevel(4);
    // Already in type
    // FastLED.setCorrection(LEDColorCorrection::Typical8mmPixel);
    leds->ClearTo(RgbColor(0, 0, 0));
    leds->Show();
    lastUpdate = millis();
    lastFpsCheck = lastUpdate;
    fpsCounter = 0;
    ledBackup.fill_solid(CRGB::Black);

    // Init random seed
#if defined(ESP32)
    randomSeed(ESP.getVcc() * analogRead(A0));
#elif defined(ESP8266)
    randomSeed(RANDOM_REG32);
#else
    randomSeed(analogRead(A0) * 17 + 23);
#endif

    colors.initRandomColors();
    colors.setSelection(0);
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
    lights["color"] = colors.getSelection();
    // TODO: cache values that do not change, reserve array space for fixed size
    JsonArray colors = lights.createNestedArray("colors");
    for (uint8_t i = 0; i < TreeColors::getSelectionCount(); ++i)
    {
        colors.add(TreeColors::getSelectionName(i));
    }
}

void TreeLight::nextEffect()
{
    DEBUGLN("Next effect");
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
    DEBUGLN("Set effect");
    if (e != currentEffectType && e < EffectType::maxValue)
    {
        currentEffectType = e;
        currentEffect = effectList[(int)e];
        resetEffect(false);
    }
}

void TreeLight::nextSpeed()
{
    DEBUGLN("Next speed");
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
    DEBUGLN("Set speed");
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
        show();
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
    show();
}

void TreeLight::resetEffect(bool timerOnly)
{
    effectTime = 0;
    // Save last effect colors
    for (int i = 0; i < numLeds; ++i)
    {
        ledBackup[i] = toCRGB(leds->GetPixelColor(i));
    }
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
    leds->SetLuminance(scale);
}

void TreeLight::initColorMenu()
{
    menuTime = millis();
    menu->setMenuState(Menu::MenuState::colorSelect);
    menu->setNumSubSelections(8);
    menu->setSubSelection(colors.getSelection());
}

unsigned int TreeLight::getFPS()
{
    unsigned long now = millis();
    unsigned int res = fpsCounter * 1000 / (now - lastFpsCheck);
    lastFpsCheck = now;
    fpsCounter = 0;
    return res;
}

void TreeLight::show(bool dithering)
{
    if (leds->CanShow())
    {
        fpsCounter++;
        leds->Show();
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
        c = currentEffect->runEffect(v, *leds, effectTime);
    }

    if (speed > 0 && c.fadeOver && effectTime < startFadeIn)
    {
        // TODO: does not work when speed = 0
        // Scale 0 to startFadeIn
        uint8_t fade = min((startFadeIn - effectTime) * 256 / startFadeIn, (unsigned long)255);
        fade = ease8InOutCubic(fade);
        blendPixels(*leds, ledBackup, fade);
    }
    else if (c.allowAutoColorChange && effectTime > colorDuration)
    {
        // 30 seconds at normal speed
        resetEffect();
        colors.updateColor();
    }
}

void TreeLight::displayMenu()
{
    leds->ClearTo(toRgb(CRGB::Black));
    CRGB color = CRGB(255, 255, 255);
    if (menu->getMenuState() == Menu::MenuState::mainSelect)
    {
        switch (menu->getLongPressMode())
        {
        case 1:
            leds->SetPixelColor(12, toRgb(color));
            break;
        case 2:
            leds->ClearTo(toRgb(color), 8, 11);
            break;
        case 3:
            leds->ClearTo(toRgb(color), 0, 7);
            break;
        case 4:
            leds->ClearTo(toRgb(CRGB::Blue), 0, 7);
            break;
        }
    }
    else if (menu->getMenuState() == Menu::MenuState::brightnessSelect)
    {
        leds->ClearTo(toRgb(color), 0, brightnessLevel - 1);
    }
    else if (menu->getMenuState() == Menu::MenuState::colorSelect)
    {
        unsigned long t = millis();
        // Show color
        if (colors.getSelection() != menu->getSubSelection())
        {
            setColorSelection(menu->getSubSelection());
            menuTime = t;
        }
        else if (t - menuTime > 1000)
        {
            menuTime = t;
            colors.updateColor();
        }
        leds->SetPixelColor(12, toRgb(colors.firstColor()));
        if (colors.isColorPalette())
        {
            uint8_t mix = (t / 64);
            for (uint8_t i = 8; i < 12; ++i)
            {
                leds->SetPixelColor(i, toRgb(colors.getPaletteColor(mix, false)));
                mix += 64;
            }
        }
        else
        {
            leds->ClearTo(toRgb(colors.secondColor()), 8, 11);
        }
        leds->SetPixelColor(colors.getSelection(), toRgb(CRGB::White));
    }
}
