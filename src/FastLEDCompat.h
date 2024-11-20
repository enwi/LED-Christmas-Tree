#ifndef FAST_LED_COMPAT_H
#define FAST_LED_COMPAT_H

#include <FastLED.h>
#include <Constants.h>
#include <NeoPixelBusLg.h>

// NeoPixelBusLg that keeps track of the original colors
template <typename T_COLOR_FEATURE, typename T_METHOD, typename T_GAMMA = NeoGammaEquationMethod>
class CustomNeoPixelBusLg : public NeoPixelBusLg<T_COLOR_FEATURE, T_METHOD, T_GAMMA>
{
public:
    using ColorObject = typename T_COLOR_FEATURE::ColorObject;
    using Base = NeoPixelBusLg<T_COLOR_FEATURE, T_METHOD, T_GAMMA>;
    CustomNeoPixelBusLg(uint16_t countPixels, uint8_t pin) : Base(countPixels, pin), buffer(countPixels, 1) { }
    CustomNeoPixelBusLg(uint16_t countPixels) : Base(countPixels), buffer(countPixels, 1) { }

    void SetPixelColor(uint16_t indexPixel, ColorObject color)
    {
        buffer.SetPixelColor(indexPixel, 0, color);
        Base::SetPixelColor(indexPixel, color);
    }
    void ClearTo(ColorObject color)
    {
        buffer.ClearTo(color);
        Base::ClearTo(color);
    }
    void ClearTo(ColorObject color, uint16_t first, uint16_t last)
    {
        for (uint16_t i = first; i <= last; ++i)
        {
            buffer.SetPixelColor(i, 0, color);
            
        }
        Base::ClearTo(color, first, last);
    }
    ColorObject GetPixelColor(uint16_t pixelIndex) const { return buffer.GetPixelColor(pixelIndex, 0); }

private:
    NeoBuffer<NeoBufferMethod<T_COLOR_FEATURE>> buffer;
};

#if defined(ESP32)
using PixelBus = CustomNeoPixelBusLg<NeoRgbFeature, NeoApa106Method, NeoGammaTableMethod>;
#elif defined(ESP8266)
using PixelBus = CustomNeoPixelBusLg<NeoRgbFeature, NeoEsp8266DmaApa106Method, NeoGammaTableMethod>;
#else
using PixelBus = CustomNeoPixelBusLg<NeoGrbFeature, NeoWs2812Method>;
#endif

inline std::vector<CRGB>& pixelBuffer(const PixelBus& leds)
{
    static std::vector<CRGB> pixels; // Static to prevent constant reallocation
    pixels.resize(leds.PixelCount());
    return pixels;
}

inline CRGB toCRGB(const RgbColor& rgb)
{
    return CRGB(rgb.R, rgb.G, rgb.B);
}

inline RgbColor toRgb(const CRGB& rgb)
{
    return RgbColor(rgb.r, rgb.g, rgb.b);
}
inline RgbColor toRgb(const CHSV& hsv)
{
    CRGB rgb = hsv;
    return toRgb(rgb);
}

inline void blendPixels(PixelBus& leds, CRGB* ledBackup, uint8_t fade)
{
    for (int i = 0; i < leds.PixelCount(); ++i)
    {
        CRGB existing = toCRGB(leds.GetPixelColor(i));
        leds.SetPixelColor(i, toRgb(existing));
    }
}

inline void fillRainbow(PixelBus& leds, uint8_t start, uint8_t end, uint8_t hue, uint8_t deltaHue)
{
    auto& pixels = pixelBuffer(leds);
    fill_rainbow(pixels.data() + start, end - start + 1, hue, deltaHue);
    for (int i = start; i <= end; ++i)
    {
        leds.SetPixelColor(i, toRgb(pixels[i]));
    }
}

inline void fillGradientRgb(PixelBus& leds, uint8_t start, uint8_t end, CRGB cStart, CRGB cEnd)
{
    auto& pixels = pixelBuffer(leds);
    fill_gradient_RGB(pixels.data(), start, cStart, end, cEnd);
    for (uint8_t i = start; i <= end; ++i)
    {
        leds.SetPixelColor(i, toRgb(pixels[i]));
    }
}

#endif