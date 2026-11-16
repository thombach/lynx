#include "ScrollText.h"
#include <Fonts/Picopixel.h>

namespace
{
    constexpr uint32_t PAUSE_MS = 1500;
    constexpr uint32_t BOUNCE_STEP_MS = 80;  // Per pixel
    constexpr uint32_t MARQUEE_STEP_MS = 40; // Per pixel, faster for long texts
    constexpr int16_t MARQUEE_GAP = 16;

    constexpr uint8_t SMALL_HEIGHT = 7, SMALL_BASELINE = 5;
    constexpr uint8_t LARGE_HEIGHT = 8;
}

ScrollText::ScrollText(uint16_t width, Font font, Mode mode)
    : width(width), font(font), mode(mode),
      canvas(width, font == Font::Small ? SMALL_HEIGHT : LARGE_HEIGHT)
{
    canvas.setTextWrap(false);
    canvas.setFont(font == Font::Small ? &Picopixel : nullptr);
}

int16_t ScrollText::baseline() const
{
    return font == Font::Small ? SMALL_BASELINE : 0; // The classic font is anchored at its top
}

int16_t ScrollText::measure(const String &text)
{
    int16_t x1, y1;
    uint16_t w, h;
    canvas.getTextBounds(text, 0, baseline(), &x1, &y1, &w, &h);
    return x1 + w;
}

int16_t ScrollText::scrollOffset(int16_t overflow, int16_t textWidth) const
{
    uint32_t elapsed = millis() - startedAt;
    if (elapsed < PAUSE_MS)
    {
        return 0;
    }
    elapsed -= PAUSE_MS;

    if (mode == Mode::Marquee)
    {
        return (elapsed / MARQUEE_STEP_MS) % (textWidth + MARQUEE_GAP);
    }

    // Scroll to the end, pause, jump back and pause again
    uint32_t cycle = overflow * BOUNCE_STEP_MS + 2 * PAUSE_MS;
    uint32_t t = elapsed % cycle;
    return min<int16_t>(t / BOUNCE_STEP_MS, overflow);
}

void ScrollText::draw(DisplayManager &display, const String &text, int16_t x, int16_t top, uint16_t color, bool center)
{
    canvas.fillScreen(0);
    int16_t textWidth = measure(text);
    int16_t overflow = textWidth - width;

    if (overflow <= 0)
    {
        canvas.setCursor(center ? -overflow / 2 : 0, baseline());
        canvas.print(text);
    }
    else
    {
        int16_t offset = scrollOffset(overflow, textWidth);
        canvas.setCursor(-offset, baseline());
        canvas.print(text);
        if (mode == Mode::Marquee)
        {
            // Second copy following the first one, so the loop is seamless
            canvas.setCursor(textWidth + MARQUEE_GAP - offset, baseline());
            canvas.print(text);
        }
    }

    display.gfx().drawBitmap(x, top, canvas.getBuffer(), width, canvas.height(), color);
}
