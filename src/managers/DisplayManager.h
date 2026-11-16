#pragma once
#include <Arduino.h>
#include <Adafruit_Protomatter.h>

class DisplayManager
{
public:
    void begin(Adafruit_Protomatter &matrix, uint8_t bitDepth);
    Adafruit_Protomatter &gfx() { return *matrix; }

    void setBrightness(float value); // 0.0 (off) to 1.0
    uint16_t color(uint8_t r, uint8_t g, uint8_t b) const;
    void drawBitmap(const uint16_t *bitmap, int16_t w, int16_t h, int16_t x, int16_t y);

    void clear();
    void show();
    // Centered message on one or two lines, used for boot, errors and empty states
    void drawMessage(const char *line1, const char *line2 = nullptr);
    void drawStaleMarker();
    void drawBrightnessOverlay(float level);

private:
    Adafruit_Protomatter *matrix = nullptr;
    float brightness = -1.0f;
    uint8_t levelStep = 1; // Smallest 8-bit channel value the matrix can show
    uint8_t lut[256];

    uint16_t scale565(uint16_t color) const;
    void drawCentered(const char *text, int16_t baseline);
};
