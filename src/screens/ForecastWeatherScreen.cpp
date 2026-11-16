#include "ForecastWeatherScreen.h"
#include <Fonts/Picopixel.h>
#include "WeatherIcons.h"
#include "core/Color.h"

namespace
{
    constexpr int16_t COLUMN_WIDTH = 32;
    constexpr int16_t DAY_BASELINE = 6;
    constexpr int16_t ICON_TOP = 8;
    constexpr int16_t TEMPERATURE_BASELINE = 29;
    constexpr int16_t TEMPERATURE_GAP = 3; // Between min and max

    String temperatureText(float value)
    {
        return String((int)roundf(value));
    }
}

void ForecastWeatherScreen::draw()
{
    if (!weather.hasData())
    {
        display.drawMessage("Loading...", "forecast");
        return;
    }

    WeatherData data = weather.get();
    Adafruit_Protomatter &gfx = display.gfx();
    struct tm now;
    bool hasTime = time.now(now);

    gfx.setFont(&Picopixel);
    gfx.setTextSize(1);

    // One column per day: tomorrow on the left, the day after on the right
    for (int day = 0; day < 2; day++)
    {
        const DailyForecast &forecast = data.days[day];
        const int16_t left = day * COLUMN_WIDTH;

        if (hasTime)
        {
            String name = TimeManager::weekdayName(now.tm_wday + day + 1);
            gfx.setTextColor(display.color(120, 120, 120));
            gfx.setCursor(left + (COLUMN_WIDTH - textWidth(name)) / 2, DAY_BASELINE);
            gfx.print(name);
        }

        display.drawBitmapCentered(weatherIcon(forecast.weatherCode, false), left, ICON_TOP, COLUMN_WIDTH,
                                   WEATHER_ICON_SLOT);

        // "min° max°" on one line, centered
        int16_t minWidth = temperatureWidth(forecast.minTemperature);
        int16_t total = minWidth + TEMPERATURE_GAP + temperatureWidth(forecast.maxTemperature);
        int16_t x = left + (COLUMN_WIDTH - total) / 2;
        drawTemperature(forecast.minTemperature, x, TEMPERATURE_BASELINE);
        drawTemperature(forecast.maxTemperature, x + minWidth + TEMPERATURE_GAP, TEMPERATURE_BASELINE);
    }

    if (weather.isStale())
    {
        display.drawStaleMarker();
    }
}

int16_t ForecastWeatherScreen::textWidth(const String &text)
{
    int16_t x1, y1;
    uint16_t w, h;
    display.gfx().getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    return x1 + w;
}

// Digits, a one-pixel gap and the one-pixel degree sign
int16_t ForecastWeatherScreen::temperatureWidth(float value)
{
    return textWidth(temperatureText(value)) + 2;
}

void ForecastWeatherScreen::drawTemperature(float value, int16_t x, int16_t baseline)
{
    Adafruit_Protomatter &gfx = display.gfx();
    uint16_t color = display.color(core::temperatureColor(value));
    gfx.setTextColor(color);
    gfx.setCursor(x, baseline);
    gfx.print(temperatureText(value));
    gfx.drawPixel(gfx.getCursorX(), baseline - 4, color); // Degree sign
}
