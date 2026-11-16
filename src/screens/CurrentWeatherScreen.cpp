#include "CurrentWeatherScreen.h"
#include <Fonts/Picopixel.h>
#include "WeatherIcons.h"

namespace
{
    constexpr int RIGHT_MARGIN = 10;
    constexpr int UNIT_WIDTH = 12;
}

void CurrentWeatherScreen::draw()
{
    if (!weather.hasData())
    {
        display.drawMessage("Loading...", "weather");
        return;
    }

    WeatherData data = weather.get();
    Adafruit_Protomatter &gfx = display.gfx();
    uint16_t white = display.color(255, 255, 255);
    int16_t x1, y1;
    uint16_t w, h;

    display.drawBitmapCentered(weatherIcon(data.weatherCode, !data.isDay), 9, 7, WEATHER_ICON_SLOT, WEATHER_ICON_SLOT);

    // Temperature
    String temperature = String((int)roundf(data.temperature));
    gfx.setFont(nullptr);
    gfx.setTextSize(1);
    gfx.getTextBounds(temperature, 0, 0, &x1, &y1, &w, &h);
    gfx.setCursor(gfx.width() - RIGHT_MARGIN - (w + UNIT_WIDTH), 11 - h / 2 - y1);
    gfx.setTextColor(white);
    gfx.print(temperature);
    int16_t x = gfx.getCursorX();
    int16_t y = gfx.getCursorY();
    gfx.drawCircle(x + 3, y, 1, white);
    gfx.setCursor(x + 6, y);
    gfx.print("C");

    // Air quality
    bool hasAqi = airQuality.hasData();
    int aqi = airQuality.get().aqi;
    String aqiStr = hasAqi ? String(aqi) : String("--");
    gfx.getTextBounds(aqiStr, 0, 0, &x1, &y1, &w, &h);
    gfx.setCursor(gfx.width() - RIGHT_MARGIN - (w + UNIT_WIDTH), 21 - h / 2 - y1);
    gfx.setTextColor(hasAqi ? display.color(core::aqiColor(aqi)) : white);
    gfx.print(aqiStr);

    gfx.setFont(&Picopixel);
    gfx.setTextColor(white);
    gfx.setCursor(gfx.getCursorX() + 2, gfx.getCursorY());
    gfx.print("AQI");

    if (weather.isStale() || airQuality.isStale())
    {
        display.drawStaleMarker();
    }
}
