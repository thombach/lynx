#include "ScrollText.h"
#include <Fonts/Picopixel.h>

namespace
{
    constexpr uint32_t PAUSE_MS = 1500;
    constexpr uint32_t STEP_MS = 80; // Per pixel
}

void ScrollText::draw(DisplayManager &display, const String &text, int16_t x, int16_t baseline, uint16_t color)
{
    canvas.fillScreen(0);
    canvas.setFont(&Picopixel);
    canvas.setTextWrap(false);

    int16_t x1, y1;
    uint16_t w, h;
    canvas.getTextBounds(text, 0, ASCENT, &x1, &y1, &w, &h);
    int overflow = x1 + w - width;

    int offset = 0;
    if (overflow > 0)
    {
        // Pause, scroll to the end, pause, jump back
        uint32_t cycle = 2 * PAUSE_MS + overflow * STEP_MS;
        uint32_t t = (millis() - startedAt) % cycle;
        offset = t < PAUSE_MS ? 0 : min<int>((t - PAUSE_MS) / STEP_MS, overflow);
    }

    canvas.setCursor(-offset, ASCENT);
    canvas.print(text);
    display.gfx().drawBitmap(x, baseline - ASCENT, canvas.getBuffer(), width, HEIGHT, color);
}
