#include "TreeColors.h"

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
            Holly_Green, Holly_Green, Holly_Green, Holly_Green, Holly_Red, Holly_Red, Holly_Red, Holly_Red};

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
        0x304048, 0x304048, 0x304048, 0x304048, 0x304048, 0x304048, 0xE0F0FF, 0xE0F0FF, 0xE0F0FF, 0xE0F0FF};

    // A palette reminiscent of large 'old-school' C9-size tree lights
    // in the five classic colors: red, orange, green, blue, and white.
    constexpr uint32_t C9_Red = 0xB80400;
    constexpr uint32_t C9_Orange = 0x902C02;
    constexpr uint32_t C9_Green = 0x046002;
    constexpr uint32_t C9_Blue = 0x070758;
    constexpr uint32_t C9_White = 0x606820;
    const TProgmemRGBPalette16 RetroC9_p FL_PROGMEM = {C9_Red, C9_Orange, C9_Red, C9_Orange, C9_Orange, C9_Red,
        C9_Orange, C9_Red, C9_Green, C9_Green, C9_Green, C9_Green, C9_Blue, C9_Blue, C9_Blue, C9_White};

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
    ///@param color Color to generate harmonic color to (using its hue)
    ///@param offsetAngle1 [0-255]
    ///@param offsetAngle2 [0-255]
    ///@param rangeAngle0 [0-255] Hue angle of allowed adjacent colors
    ///@param rangeAngle1 [0-255]
    ///@param rangeAngle2 [0-255]
    ///@param saturation Saturation of the generated color [0-255]
    ///@param luminance Luminance of the generated color [0-255]
    ///@return CRGB
    CRGB GenerateHarmonicColor(const CHSV color, uint8_t offsetAngle1, uint8_t offsetAngle2, uint8_t rangeAngle0,
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

    const TProgmemRGBPalette16* palettes[] = {nullptr, // harmonic colors
        nullptr, //
        &Holly_p, //
        &RetroC9_p, //
        &FairyLight_p, //
        &Snow_p, //
        nullptr, //
        nullptr};
    constexpr uint8_t paletteCount = sizeof(palettes) / sizeof(TProgmemRGBPalette16*);
    const char* paletteNames[paletteCount] = {"Random Rainbow", //
        "Random Pastel", //
        "Holly", //
        "RetroC9", //
        "FairyLight", //
        "Snow", //
        "Reds", //
        "Greens"};

} // namespace

void TreeColors::initRandomColors()
{
    color1 = CRGB(random(0, 255), random(0, 255), random(0, 255));
    color2 = CRGB(random(0, 255), random(0, 255), random(0, 255));
}

void TreeColors::setSelection(uint8_t index)
{
    if (index != selection)
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
        selection = index;
        updateColor();
        updateColor();
    }
}

void TreeColors::updateColor()
{
    color1 = color2;

    CRGB colorDifference;
    for (uint8_t i = 0; i < 4; ++i)
    {
        if (isColorPalette())
        {
            color2 = ColorFromPalette(currentPalette, random(0, 255));
        }
        else
        {
            color2 = generateColor(selection, color2);
        }
        colorDifference = color1;
        colorDifference -= color2;
        if (colorDifference.getAverageLight() > 8)
        {
            return;
        }
    }
}

bool TreeColors::isColorPalette() const
{
    return getPaletteSelection(selection) != nullptr;
}

CRGB TreeColors::getPaletteColor(uint8_t mix, bool doBlend) const
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
                return color1;
            }
            else
            {
                return color2;
            }
        }
        else
        {
            return blend(color1, color2, mix);
        }
    }
}

CRGB TreeColors::generateColor(uint8_t selection, CRGB baseColor)
{
    switch (selection)
    {
    case 0:
        // Random rainbow
        return GenerateHarmonicColor(rgb2hsv_approximate(baseColor), 16, 32, 8, 16, 32, 255, 255);
    case 1:
        // Random pastel
        return GenerateHarmonicColor(rgb2hsv_approximate(baseColor), 16, 32, 8, 16, 32, 128, 255);
    case 6: {
        // Reds
        // TODO: Range is only applied towards positive hues, which is why the starting color is shifted here
        uint8_t brightness = random(200, 255);
        return GenerateHarmonicColor(CHSV(HUE_RED - 16, 255, 255), 0, 0, 32, 0, 0, 255, brightness);
    }
    case 7: {
        // Greens
        uint8_t brightness = random(200, 255);
        return GenerateHarmonicColor(CHSV(HUE_GREEN - 16, 255, 255), 0, 0, 32, 0, 0, 255, brightness);
    }
    default:
        return CRGB::Black;
    }
}

const TProgmemRGBPalette16* TreeColors::getPaletteSelection(uint8_t i)
{
    if (i < paletteCount)
    {
        return palettes[i];
    }
    return nullptr;
}

const char* TreeColors::getSelectionName(uint8_t i)
{
    if (i < paletteCount)
    {
        return paletteNames[i];
    }
    return "";
}

uint8_t TreeColors::getSelectionCount()
{
    return paletteCount;
}
