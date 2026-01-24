#include "DisplayManager.h"
#include <math.h>

DisplayManager displayManager;

DisplayManager::DisplayManager()
{
    matrix = nullptr;
    brightness = 1.0f;
}

void DisplayManager::begin(Adafruit_Protomatter *m)
{
    matrix = m;
    brightness = -1.0f; // force initial rebuild
    updateBrightness();
}

void DisplayManager::updateBrightness()
{
    float newBrightness = ConfigManager::getInstance().brightness;

    // Clamp
    if (newBrightness < 0.0f)
        newBrightness = 0.0f;
    if (newBrightness > 1.0f)
        newBrightness = 1.0f;

    // Only rebuild LUTs if brightness actually changed
    if (fabsf(newBrightness - brightness) > 0.001f)
    {
        brightness = newBrightness;
        buildLUTs();
    }
}
//
// Build Gamma & Brightness LUTs
//
void DisplayManager::buildLUTs()
{
    const float gamma = 2.2f;

    for (int i = 0; i < 256; i++)
    {
        float normalized = (float)i / 255.0f;

        // Gamma correction forward
        float g = powf(normalized, gamma);

        // Brightness scaling applied TO gamma-corrected value
        g *= brightness;
        if (g < 0)
            g = 0;
        if (g > 1)
            g = 1;

        // Gamma correction inverse
        uint8_t out = (uint8_t)(powf(g, 1.0f / gamma) * 255.0f);

        gammaLUT[i] = out;
    }

    // Brightness-only LUT for bitmap RGB565 channels (0–255)
    for (int i = 0; i < 256; i++)
    {
        float scaled = (float)i * brightness;
        if (scaled > 255)
            scaled = 255;
        brightnessLUT[i] = (uint8_t)scaled;
    }
}

//
// Convert 24-bit RGB to 16-bit RGB565
//
uint16_t DisplayManager::rgbTo565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) |
           ((g & 0xFC) << 3) |
           (b >> 3);
}

uint16_t DisplayManager::color565(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t rr = gammaLUT[r];
    uint8_t gg = gammaLUT[g];
    uint8_t bb = gammaLUT[b];

    return rgbTo565(rr, gg, bb);
}

//
// Draw a bitmap using LUT brightness scaling
// (Bitmap must be 565 format)
//
void DisplayManager::drawBitmap(const uint16_t *bitmap, int w, int h, int x, int y)
{
    uint16_t *scaled = new uint16_t[w * h];

    for (int i = 0; i < w * h; i++)
    {
        uint16_t c = bitmap[i];

        // Extract 565 colors
        uint8_t r5 = (c >> 11) & 0x1F;
        uint8_t g6 = (c >> 5) & 0x3F;
        uint8_t b5 = c & 0x1F;

        // Convert to 8-bit for LUT
        uint8_t r = r5 << 3;
        uint8_t g = g6 << 2;
        uint8_t b = b5 << 3;

        // Apply brightness LUT ONLY (bitmap does not need gamma)
        r = gammaLUT[r];
        g = gammaLUT[g];
        b = gammaLUT[b];

        scaled[i] = rgbTo565(r, g, b);
    }

    matrix->drawRGBBitmap(x, y, scaled, w, h);

    delete[] scaled;
}
