#define FASTLED_ALLOW_INTERRUPTS 0
#include <AceButton.h>
#include <FastLED.h>

#include "TreeLight.h"

using namespace ace_button;

constexpr uint8_t buttonPin = D2;
constexpr uint8_t pixelPin = D1;

// LED order:
//       USB
//        0
//    7   8   1
// 6   11 12 9   2
//    5   10   3
//        4

CRGB c1 = CRGB(0, 0xA0, 0xFF);
CRGB c2 = CRGB(0, 0x40, 0xFF);

unsigned long nextUpdate = 0;
uint8_t effect = 0;
uint8_t numEffects = 2;

AceButton button(buttonPin);
void handleButton(AceButton*, uint8_t eventType, uint8_t);
TreeLight light;

void setup()
{
    light.init();
    pinMode(buttonPin, INPUT);
    ButtonConfig* buttonConfig = button.getButtonConfig();
    buttonConfig->setEventHandler(handleButton);
    buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
    buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterDoubleClick);
    nextUpdate = millis();
}

void loop()
{
    // 1. Check button state:
    //    - Debounce
    //    - Single click / double click detection
    //    - Long hold detection
    // 2. Process button update
    // 3. Update LEDs (if necessary)
    // 4. wait until next update / button poll

    // 1. & 2.
    button.check();

    // 3.
    light.update();
}

void handleButton(AceButton*, uint8_t eventType, uint8_t)
{
    switch (eventType)
    {
    case AceButton::kEventClicked:
    case AceButton::kEventReleased:
        light.nextEffect();
        break;
    case AceButton::kEventDoubleClicked:
        // double click
        break;
    case AceButton::kEventLongPressed:

        break;
    }
}
