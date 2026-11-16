#include "DisplayManager.h"
#include <Fonts/Picopixel.h>
#include <math.h>

namespace
{
    constexpr float GAMMA = 2.2f;

    uint16_t pack565(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
}

void DisplayManager::begin(Adafruit_Protomatter &m, uint8_t bitDepth)
{
    matrix = &m;
    levelStep = 256 >> bitDepth; // Protomatter keeps only the top bitDepth bits of each channel
    matrix->setTextWrap(false);
    setBrightness(1.0f);
}

void DisplayManager::setBrightness(float value)
{
    value = constrain(value, 0.0f, 1.0f);
    if (fabsf(value - brightness) < 0.001f)
    {
        return;
    }
    brightness = value;

    // Perceptual dimming: brightness acts on lightness rather than raw LED duty
    float factor = powf(brightness, 1.0f / GAMMA);
    lut[0] = 0;
    for (int i = 1; i < 256; i++)
    {
        // Round to the nearest level the matrix can show (it would truncate otherwise),
        // and keep lit pixels at least one level up so they never vanish when dimmed
        int level = lroundf(i * factor / levelStep) * levelStep;
        lut[i] = constrain(level, brightness > 0 ? levelStep : 0, 255);
    }
}

uint16_t DisplayManager::color(uint8_t r, uint8_t g, uint8_t b) const
{
    return pack565(lut[r], lut[g], lut[b]);
}

uint16_t DisplayManager::scale565(uint16_t c) const
{
    return color(((c >> 11) & 0x1F) << 3, ((c >> 5) & 0x3F) << 2, (c & 0x1F) << 3);
}

void DisplayManager::drawBitmap(const uint16_t *bitmap, int16_t w, int16_t h, int16_t x, int16_t y)
{
    matrix->startWrite();
    for (int16_t row = 0; row < h; row++)
    {
        for (int16_t col = 0; col < w; col++)
        {
            uint16_t c = pgm_read_word(&bitmap[row * w + col]);
            if (c)
            {
                matrix->writePixel(x + col, y + row, scale565(c));
            }
        }
    }
    matrix->endWrite();
}

void DisplayManager::clear()
{
    matrix->fillScreen(0);
}

void DisplayManager::show()
{
    matrix->show();
}

void DisplayManager::drawCentered(const char *text, int16_t baseline)
{
    int16_t x1, y1;
    uint16_t w, h;
    matrix->getTextBounds(text, 0, baseline, &x1, &y1, &w, &h);
    matrix->setCursor((matrix->width() - w) / 2 - x1, baseline);
    matrix->print(text);
}

void DisplayManager::drawMessage(const char *line1, const char *line2)
{
    matrix->setFont(&Picopixel);
    matrix->setTextSize(1);
    matrix->setTextColor(color(255, 255, 255));
    if (line2)
    {
        drawCentered(line1, 13);
        matrix->setTextColor(color(255, 128, 0));
        drawCentered(line2, 23);
    }
    else
    {
        drawCentered(line1, 18);
    }
}

void DisplayManager::drawStaleMarker()
{
    matrix->drawPixel(matrix->width() - 1, 0, color(255, 0, 0));
}

// Bottom banner with the brightness percentage and a gauge, drawn over the current screen
void DisplayManager::drawBrightnessOverlay(float level)
{
    constexpr int16_t TOP = 23;
    constexpr int16_t BAR_X = 20, BAR_Y = 26, BAR_W = 41, BAR_H = 4;
    int16_t width = matrix->width();

    matrix->fillRect(0, TOP, width, matrix->height() - TOP, 0);
    matrix->drawFastHLine(0, TOP, width, color(60, 60, 60));

    matrix->setFont(&Picopixel);
    matrix->setTextSize(1);
    matrix->setTextColor(color(255, 255, 255));
    matrix->setCursor(2, 30);
    matrix->print(String(lroundf(level * 100)) + "%");

    matrix->drawRect(BAR_X, BAR_Y, BAR_W, BAR_H, color(90, 90, 90));
    matrix->fillRect(BAR_X + 1, BAR_Y + 1, lroundf((BAR_W - 2) * level), BAR_H - 2, color(255, 200, 0));
}
