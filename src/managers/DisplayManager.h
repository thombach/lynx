#pragma once
#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include "bitmap.h"
#include "core/Color.h"

class DisplayManager
{
public:
    void begin(Adafruit_Protomatter &matrix, uint8_t bitDepth);
    Adafruit_Protomatter &gfx() { return *matrix; }

    void setBrightness(float value); // 0.0 (off) to 1.0
    uint16_t color(uint8_t r, uint8_t g, uint8_t b) const;
    uint16_t color(const core::Rgb &rgb) const { return color(rgb.r, rgb.g, rgb.b); }
    void drawBitmap(const Bitmap &bitmap, int16_t x, int16_t y);
    // Centered in a w x h slot, so icons of different sizes share the same layout
    void drawBitmapCentered(const Bitmap &bitmap, int16_t x, int16_t y, int16_t w, int16_t h);

    void clear();
    void show();
    // Centered message on one or two lines, used for boot, errors and empty states
    void drawMessage(const char *line1, const char *line2 = nullptr);
    void drawStaleMarker();
    void drawPauseMarker();
    // Text centered horizontally with the current font and color
    void drawCentered(const char *text, int16_t baseline);
    void drawBrightnessOverlay(float level);
    void drawBanner(const char *text);

private:
    Adafruit_Protomatter *matrix = nullptr;
    float brightness = -1.0f;
    uint8_t levelStep = 1; // Smallest 8-bit channel value the matrix can show
    uint8_t lut[256];

    uint16_t scale565(uint16_t color) const;
    void clearBanner();
};
