#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "managers/DisplayManager.h"

// Single line of Picopixel text clipped to a fixed width, scrolling back and forth
// when it doesn't fit.
class ScrollText
{
public:
    explicit ScrollText(uint16_t width) : width(width), canvas(width, HEIGHT) {}

    void reset() { startedAt = millis(); }
    void draw(DisplayManager &display, const String &text, int16_t x, int16_t baseline, uint16_t color);

private:
    static constexpr uint8_t HEIGHT = 7; // Picopixel line height
    static constexpr uint8_t ASCENT = 5;

    const uint16_t width;
    GFXcanvas1 canvas;
    uint32_t startedAt = 0;
};
