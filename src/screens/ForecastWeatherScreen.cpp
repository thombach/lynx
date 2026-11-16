#include "ForecastWeatherScreen.h"
#include <Fonts/Picopixel.h>
#include "WeatherIcons.h"

void ForecastWeatherScreen::draw()
{
    if (!weather.hasData())
    {
        display.drawMessage("Loading...", "forecast");
        return;
    }

    WeatherData data = weather.get();
    Adafruit_Protomatter &gfx = display.gfx();
    uint16_t white = display.color(255, 255, 255);
    uint16_t minColor = display.color(0, 207, 255);
    uint16_t maxColor = display.color(255, 64, 0);
    struct tm now;
    bool hasTime = time.now(now);

    gfx.setFont(&Picopixel);
    gfx.setTextSize(1);

    // One row per day: today on top, tomorrow below
    for (int day = 0; day < 2; day++)
    {
        const DailyForecast &forecast = data.days[day];
        int16_t top = day * 16;

        display.drawBitmap(weatherIcon(forecast.weatherCode, false), 16, 16, 2, top);

        if (hasTime)
        {
            gfx.setTextColor(white);
            gfx.setCursor(20, top + 10);
            gfx.print(TimeManager::weekdayName(now.tm_wday + day));
        }

        drawTemperature(forecast.minTemperature, 36, top + 6, minColor);
        drawTemperature(forecast.maxTemperature, 36, top + 13, maxColor);

        gfx.setTextColor(white);
        gfx.setCursor(47, top + 10);
        gfx.print(String((int)roundf(forecast.precipitationSum)) + " m");
    }

    if (weather.isStale())
    {
        display.drawStaleMarker();
    }
}

void ForecastWeatherScreen::drawTemperature(float value, int16_t x, int16_t y, uint16_t color)
{
    Adafruit_Protomatter &gfx = display.gfx();
    gfx.setTextColor(color);
    gfx.setCursor(x, y);
    gfx.print((int)roundf(value));
    gfx.drawPixel(gfx.getCursorX(), y - 4, color); // Degree sign
}
