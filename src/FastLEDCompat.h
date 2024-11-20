#ifndef FAST_LED_COMPAT_H
#define FAST_LED_COMPAT_H

#include <FastLED.h>
#include <NeoPixelBusLg.h>

#if defined(ESP32)
using PixelBus = NeoPixelBusLg<NeoRgbFeature, NeoApa106Method>;
#elif defined(ESP8266)
using PixelBus = NeoPixelBusLg<NeoRgbFeature, NeoEsp8266DmaApa106Method>;
#else
using PixelBus = NeoPixelBusLg<NeoGrbFeature, NeoWs2812Method>;
#endif

inline std::vector<CRGB>& pixelBuffer(const PixelBus& leds){
    static std::vector<CRGB> pixels; // Static to prevent constant reallocation
    pixels.resize(leds.PixelCount());
    return pixels;

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

inline void fillRainbow(PixelBus& leds, uint8_t start, uint8_t end, uint8_t hue, uint8_t deltaHue)
{
    auto& pixels = pixelBuffer(leds);
    fill_rainbow(pixels.data() + start, end-start+1, hue, deltaHue);
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