#include "BootAnimation.h"
#include <math.h>
#include "core/Color.h"

namespace
{
    constexpr uint32_t FRAME_MS = 40;

    const char TEXT[] = "Lynx";
    constexpr int LETTERS = 4;
    const core::Rgb COLORS[LETTERS] = {{0, 255, 64}, {255, 128, 0}, {0, 64, 255}, {255, 0, 255}};

    // Built-in 5x7 font: 6 px per character, 8 px tall
    constexpr int16_t CHAR_W = 6, CHAR_H = 8;
    constexpr int16_t TEXT_W = CHAR_W * LETTERS;
    constexpr int16_t TOP = 11; // Text and underline centered vertically
    constexpr int16_t LINE_Y = TOP + CHAR_H + 2;
    constexpr float LINE_DIM = 0.6f;

    // Timeline (ms)
    constexpr uint32_t DROP_STAGGER = 110, DROP_MS = 450;
    constexpr uint32_t LINE_START = 700, LINE_MS = 300;
    constexpr uint32_t SHINE_START = 950, SHINE_MS = 450;
    constexpr float SHINE_HALF_WIDTH = 2.0f;

    float progress(uint32_t t, uint32_t start, uint32_t duration)
    {
        if (t <= start)
            return 0.0f;
        return min(1.0f, (float)(t - start) / duration);
    }

    float easeOutCubic(float t)
    {
        return 1 - powf(1 - t, 3);
    }

    // Falls and rebounds a few times with decreasing height
    float easeOutBounce(float t)
    {
        constexpr float N = 7.5625f, D = 2.75f;
        if (t < 1 / D)
            return N * t * t;
        if (t < 2 / D)
        {
            t -= 1.5f / D;
            return N * t * t + 0.75f;
        }
        if (t < 2.5f / D)
        {
            t -= 2.25f / D;
            return N * t * t + 0.9375f;
        }
        t -= 2.625f / D;
        return N * t * t + 0.984375f;
    }

    core::Rgb scaled(const core::Rgb &c, float k)
    {
        return {(uint8_t)lroundf(c.r * k), (uint8_t)lroundf(c.g * k), (uint8_t)lroundf(c.b * k)};
    }

    core::Rgb towardWhite(const core::Rgb &c, float k)
    {
        return {(uint8_t)lroundf(c.r + (255 - c.r) * k), (uint8_t)lroundf(c.g + (255 - c.g) * k),
                (uint8_t)lroundf(c.b + (255 - c.b) * k)};
    }
}

void BootAnimation::play(DisplayManager &display)
{
    uint32_t start = millis();
    for (;;)
    {
        uint32_t frameStart = millis();
        uint32_t t = min(frameStart - start, DURATION_MS);
        drawFrame(display, t);
        display.show();
        if (t >= DURATION_MS)
        {
            return;
        }

        uint32_t elapsed = millis() - frameStart;
        if (elapsed < FRAME_MS)
        {
            delay(FRAME_MS - elapsed);
        }
    }
}

void BootAnimation::drawFrame(DisplayManager &display, uint32_t t)
{
    Adafruit_Protomatter &gfx = display.gfx();
    int16_t left = (gfx.width() - TEXT_W) / 2;

    display.clear();
    gfx.setFont(nullptr);
    gfx.setTextSize(1);

    // Letters drop from above the screen, one after another
    for (int i = 0; i < LETTERS; i++)
    {
        float k = progress(t, i * DROP_STAGGER, DROP_MS);
        if (k <= 0)
        {
            continue;
        }
        int16_t y = lroundf(-CHAR_H + (TOP + CHAR_H) * easeOutBounce(k));
        uint16_t color = display.color(COLORS[i]);
        gfx.drawChar(left + i * CHAR_W, y, TEXT[i], color, color, 1);
    }

    // Underline grows from the center, each segment in the color of the letter above
    float line = easeOutCubic(progress(t, LINE_START, LINE_MS));
    int16_t half = lroundf(line * TEXT_W / 2);
    for (int16_t x = TEXT_W / 2 - half; x < TEXT_W / 2 + half; x++)
    {
        gfx.drawPixel(left + x, LINE_Y, display.color(scaled(COLORS[x / CHAR_W], LINE_DIM)));
    }

    // Diagonal shine sweeping left to right over the lit pixels
    float shine = progress(t, SHINE_START, SHINE_MS);
    if (shine <= 0 || shine >= 1)
    {
        return;
    }
    constexpr float SLANT = 0.5f; // Horizontal offset per row
    float span = TEXT_W + (LINE_Y - TOP) * SLANT + 2 * SHINE_HALF_WIDTH;
    float center = -SHINE_HALF_WIDTH + shine * span;
    uint16_t *buffer = gfx.getBuffer();
    for (int16_t y = TOP; y <= LINE_Y; y++)
    {
        for (int16_t x = 0; x < TEXT_W; x++)
        {
            uint16_t &pixel = buffer[y * gfx.width() + left + x];
            float k = 1 - fabsf(x - (y - TOP) * SLANT - center) / SHINE_HALF_WIDTH;
            if (!pixel || k <= 0)
            {
                continue;
            }
            const core::Rgb &base = COLORS[x / CHAR_W];
            pixel = display.color(towardWhite(y == LINE_Y ? scaled(base, LINE_DIM) : base, k));
        }
    }
}
