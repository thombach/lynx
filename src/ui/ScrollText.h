#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "managers/DisplayManager.h"

// Single line of text clipped to a fixed width. Text that doesn't fit either
// bounces back and forth, or loops continuously like a news ticker.
class ScrollText
{
public:
    enum class Font
    {
        Small, // Picopixel, 7 px high
        Large, // Classic 5x7 GFX font, 8 px high
    };
    enum class Mode
    {
        Bounce,
        Marquee,
    };

    ScrollText(uint16_t width, Font font = Font::Small, Mode mode = Mode::Bounce);

    void reset() { startedAt = millis(); }
    // `top` is the first pixel row of the line; text that fits is left aligned or centered
    void draw(DisplayManager &display, const String &text, int16_t x, int16_t top, uint16_t color, bool center = false);

private:
    const uint16_t width;
    const Font font;
    const Mode mode;
    GFXcanvas1 canvas;
    uint32_t startedAt = 0;

    int16_t baseline() const;
    int16_t measure(const String &text);
    int16_t scrollOffset(int16_t overflow, int16_t textWidth) const;
};
