#include "HourlyScreen.h"
#include <Fonts/Picopixel.h>
#include "core/Color.h"
#include "core/Weather.h"

namespace
{
    constexpr int16_t MARGIN = 2;
    constexpr int16_t PLOT_TOP = 4;
    constexpr int16_t PLOT_BOTTOM = 22;
    constexpr int16_t HOURS_BASELINE = 29;
    constexpr int16_t SCALE_LEFT = 2; // Temperature labels, off the screen edge
    constexpr size_t LABELED_HOURS[] = {0, HOURLY_POINTS / 2, HOURLY_POINTS - 1};
    constexpr int16_t RAIN_BAR_WIDTH = 3;
    constexpr core::Rgb RAIN_COLOR = {0, 50, 150}; // Dim, so the curve stays in front

    String temperatureText(float value)
    {
        return String((int)roundf(value));
    }
}

void HourlyScreen::draw()
{
    if (!weather.hasData())
    {
        display.drawMessage("Loading...", "forecast");
        return;
    }

    WeatherData data = weather.get();
    Adafruit_Protomatter &gfx = display.gfx();
    const float *temps = data.hourlyTemperature;

    float minTemp = temps[0], maxTemp = temps[0];
    for (size_t i = 1; i < HOURLY_POINTS; i++)
    {
        minTemp = min(minTemp, temps[i]);
        maxTemp = max(maxTemp, temps[i]);
    }
    auto rowOf = [&](float temp)
    { return core::scaleY(temp, minTemp, maxTemp, PLOT_TOP, PLOT_BOTTOM); };

    // Scale on the left, the curve takes the remaining width
    gfx.setFont(&Picopixel);
    gfx.setTextSize(1);
    const int16_t left = SCALE_LEFT + max(labelWidth(minTemp), labelWidth(maxTemp)) + MARGIN;
    const int16_t right = gfx.width() - 1 - MARGIN;
    auto pointX = [&](size_t index)
    { return left + (int16_t)lroundf((float)index * (right - left) / (HOURLY_POINTS - 1)); };

    // Rain bars behind the curve, centered on their hour; nothing at all on dry hours
    for (size_t i = 0; i < HOURLY_POINTS; i++)
    {
        int16_t height = core::precipitationBarHeight(data.hourlyPrecipitation[i]);
        if (height > 0)
        {
            gfx.fillRect(pointX(i) - RAIN_BAR_WIDTH / 2, PLOT_BOTTOM - height + 1, RAIN_BAR_WIDTH, height,
                         display.color(RAIN_COLOR));
        }
    }

    // Curve, interpolated per column
    int16_t previousY = -1;
    for (int16_t x = left; x <= right; x++)
    {
        float position = (float)(x - left) * (HOURLY_POINTS - 1) / (right - left);
        size_t index = min((size_t)position, HOURLY_POINTS - 2);
        float temp = temps[index] + (temps[index + 1] - temps[index]) * (position - index);
        int16_t y = rowOf(temp);
        core::Rgb rgb = core::temperatureColor(temp);

        if (previousY < 0)
        {
            gfx.drawPixel(x, y, display.color(rgb));
        }
        else
        {
            gfx.drawLine(x - 1, previousY, x, y, display.color(rgb));
        }
        previousY = y;
    }

    // The curve spans the full height: max at the top, min at the bottom
    if (temperatureText(minTemp) == temperatureText(maxTemp))
    {
        drawTemperatureLabel(maxTemp, SCALE_LEFT, (PLOT_TOP + PLOT_BOTTOM) / 2); // Min and max round alike
    }
    else
    {
        drawTemperatureLabel(maxTemp, SCALE_LEFT, PLOT_TOP);
        drawTemperatureLabel(minTemp, SCALE_LEFT, PLOT_BOTTOM);
    }

    // Start, middle and end hours, aligned with the curve's ends
    gfx.setTextColor(display.color(120, 120, 120));
    for (size_t i : LABELED_HOURS)
    {
        String label = String((data.firstHour + i) % 24) + "h";
        int16_t x1, y1;
        uint16_t w, h;
        gfx.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
        int16_t x = i == 0 ? left : i == HOURLY_POINTS - 1 ? right - w + 1 : pointX(i) - w / 2;
        gfx.setCursor(x, HOURS_BASELINE);
        gfx.print(label);
    }

    if (weather.isStale())
    {
        display.drawStaleMarker();
    }
}

// Width of "12°" in Picopixel: digits plus the one-pixel degree sign
int16_t HourlyScreen::labelWidth(float value)
{
    int16_t x1, y1;
    uint16_t w, h;
    display.gfx().getTextBounds(temperatureText(value), 0, 0, &x1, &y1, &w, &h);
    return x1 + w + 2;
}

// Label vertically centered on row y
void HourlyScreen::drawTemperatureLabel(float value, int16_t x, int16_t y)
{
    Adafruit_Protomatter &gfx = display.gfx();
    uint16_t color = display.color(core::temperatureColor(value));
    int16_t baseline = y + 2; // Picopixel digits span baseline-4 to baseline
    gfx.setTextColor(color);
    gfx.setCursor(x, baseline);
    gfx.print(temperatureText(value));
    gfx.drawPixel(gfx.getCursorX(), baseline - 4, color);
}
