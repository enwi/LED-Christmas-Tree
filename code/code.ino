#define FASTLED_ALLOW_INTERRUPTS 0
#include <AceButton.h>
#include <FastLED.h>

using namespace ace_button;

constexpr uint8_t buttonPin = D2;
constexpr uint8_t pixelPin = D1;
constexpr uint8_t numLeds = 13;

CRGB leds[numLeds];
// LED order:
//       USB
//        0
//    7   8   1
// 6   11 12 9   2
//    5   10   3
//        4

CRGB c1 = CRGB(0, 0xA0, 0xFF);
CRGB c2 = CRGB(0, 0x40, 0xFF);

long nextUpdate = 0;
uint8_t effect = 0;
uint8_t numEffects = 2;

AceButton button(buttonPin);
void handleButton(AceButton *, uint8_t eventType, uint8_t);

void setup() {
  pinMode(buttonPin, INPUT);
  FastLED.addLeds<NEOPIXEL, pixelPin>(leds, numLeds);
  FastLED.setBrightness(64);
  std::fill(std::begin(leds), std::end(leds), 0);
  FastLED.show();
  ButtonConfig *buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleButton);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(
      ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterDoubleClick);
}

void loop() {
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
  long time = millis();
  if (time - nextUpdate > 0) {
    updateLEDs();
  }
}

void handleButton(AceButton *, uint8_t eventType, uint8_t) {
  switch (eventType) {
  case AceButton::kEventClicked:
  case AceButton::kEventReleased:
    // single click
    if (++effect == numEffects) {
      effect = 0;
    }
    nextUpdate = millis();
    break;
  case AceButton::kEventDoubleClicked:
    // double click
    break;
  case AceButton::kEventLongPressed:

    break;
  }
}

void updateLEDs() {
  if (effect == 0) {
    twoColorTwinkle(c1, c2);
    std::swap(c1, c2);
    nextUpdate += 1000;
  } else if (effect == 1) {
    solidColor(c1);
  }
}

// Effects
// two color change
void twoColorTwinkle(CRGB color1, CRGB color2) {
  for (uint8_t i = 0; i < numLeds; ++i) {
    if ((i & 1) == 0) {
      leds[i] = color1;
    } else {
      leds[i] = color2;
    }
  }
  FastLED.show();
}

void solidColor(CRGB color) {
  std::fill(std::begin(leds), std::end(leds), color);
  FastLED.show();
}