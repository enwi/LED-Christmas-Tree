#include "TreeLight.h"

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
    menu->setSubSelection(colors.getSelection());
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
        colors.updateColor();
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
        case 4:
            leds(0, 7) = CRGB::Blue;
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
        leds[12] = colors.firstColor();
        if (colors.isColorPalette())
        {
            uint8_t mix = (t / 64);
            for (uint8_t i = 8; i < 12; ++i)
            {
                leds[i] = colors.getPaletteColor(mix, false);
                mix += 64;
            }
        }
        else
        {
            leds(8, 11) = colors.secondColor();
        }
        leds[colors.getSelection()] = CRGB::White;
    }
}
