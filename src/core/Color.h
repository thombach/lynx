#pragma once
// Pure color helpers, without Arduino dependencies so they can be unit tested natively
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace core
{
    struct Rgb
    {
        uint8_t r, g, b;
    };

    inline bool operator==(const Rgb &a, const Rgb &b)
    {
        return a.r == b.r && a.g == b.g && a.b == b.b;
    }

    // European AQI bands
    inline Rgb aqiColor(int aqi)
    {
        if (aqi < 20)
            return {0, 255, 0}; // Good
        if (aqi < 40)
            return {255, 255, 0}; // Fair
        if (aqi < 60)
            return {255, 128, 0}; // Moderate
        if (aqi < 80)
            return {255, 0, 0}; // Poor
        if (aqi < 100)
            return {255, 0, 255}; // Very poor
        return {0, 0, 255};       // Extremely poor
    }

    // Cold blue to hot red, interpolated between fixed stops
    inline Rgb temperatureColor(float celsius)
    {
        static const struct
        {
            float t;
            Rgb color;
        } STOPS[] = {
            {-5, {80, 80, 255}},
            {5, {0, 200, 255}},
            {15, {0, 255, 100}},
            {22, {255, 220, 0}},
            {30, {255, 40, 0}},
        };
        constexpr int COUNT = sizeof(STOPS) / sizeof(STOPS[0]);

        if (celsius <= STOPS[0].t)
            return STOPS[0].color;
        for (int i = 1; i < COUNT; i++)
        {
            if (celsius <= STOPS[i].t)
            {
                float k = (celsius - STOPS[i - 1].t) / (STOPS[i].t - STOPS[i - 1].t);
                const Rgb &a = STOPS[i - 1].color;
                const Rgb &b = STOPS[i].color;
                return {(uint8_t)lroundf(a.r + (b.r - a.r) * k),
                        (uint8_t)lroundf(a.g + (b.g - a.g) * k),
                        (uint8_t)lroundf(a.b + (b.b - a.b) * k)};
            }
        }
        return STOPS[COUNT - 1].color;
    }

    // "#rrggbb" or "rrggbb"
    inline bool parseHexColor(const char *text, Rgb &out)
    {
        if (!text)
            return false;
        if (*text == '#')
            text++;
        unsigned int r, g, b;
        if (strlen(text) != 6 || sscanf(text, "%2x%2x%2x", &r, &g, &b) != 3)
            return false;
        out = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
        return true;
    }

    // Brightness LUT entry: scaled value rounded to the nearest level the matrix can show
    // (`step`), and never below one level for lit pixels so they don't vanish when dimmed
    inline uint8_t quantizedLevel(int value, float factor, int step)
    {
        if (value <= 0 || factor <= 0)
            return 0;
        long level = lroundf(value * factor / step) * step;
        if (level < step)
            level = step;
        return level > 255 ? 255 : (uint8_t)level;
    }

    // Maps value in [min, max] to a pixel row, max at `top`
    inline int scaleY(float value, float min, float max, int top, int bottom)
    {
        if (max - min < 0.01f)
            return (top + bottom) / 2;
        float k = (value - min) / (max - min);
        return (int)lroundf(bottom - k * (bottom - top));
    }
}
