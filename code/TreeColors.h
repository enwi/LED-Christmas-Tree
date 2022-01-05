#ifndef TREE_COLORS_H
#define TREE_COLORS_H

#include <FastLED.h>
#include <stdint.h>

class TreeColors
{
public:
    // Set color selection and update colors if changed
    void setSelection(uint8_t index);
    uint8_t getSelection() const { return selection; }

    // Set first color to second color and choose new second color
    void updateColor();

    CRGB firstColor() const { return color1; }
    CRGB secondColor() const { return color2; }

    bool isColorPalette() const;

    // Is palette: color from palette
    // Not a palette: between current first and second color
    CRGB getPaletteColor(uint8_t mix, bool doBlend = true) const;

    static const char* getSelectionName(uint8_t i);
    static uint8_t getSelectionCount();

private:
    // returns nullptr if selection is not a palette
    static const TProgmemRGBPalette16* getPaletteSelection(uint8_t i);
    // Generate color for selection which is not a palette
    static CRGB generateColor(uint8_t selection, CRGB baseColor);

private:
    CRGB color1 = CRGB(0, 0xA0, 0xFF);
    CRGB color2 = CRGB(0, 0x40, 0xFF);
    CRGBPalette16 currentPalette {CRGB::Black};
    uint8_t selection = 0;
};

#endif
