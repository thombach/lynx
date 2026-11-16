#include "SunScreen.h"
#include "bitmap.h"

namespace
{
    constexpr int16_t ICON_GAP = 5;
    constexpr int16_t ROW_GAP = 4;
    constexpr int16_t CHAR_WIDTH = 6, GLYPH_HEIGHT = 7; // Classic GFX font
}

void SunScreen::draw()
{
    if (!weather.hasData())
    {
        display.drawMessage("Loading...", "sun");
        return;
    }

    WeatherData data = weather.get();
    Adafruit_Protomatter &gfx = display.gfx();

    // Two rows, icon then time ("HH:MM"), centered as a block
    const Bitmap &sunrise = bitmap_sunrise;
    const Bitmap &sunset = bitmap_sunset;
    int16_t iconWidth = max(sunrise.width, sunset.width);
    int16_t textWidth = CHAR_WIDTH * max(strlen(data.sunrise), strlen(data.sunset)) - 1;
    int16_t iconX = (gfx.width() - (iconWidth + ICON_GAP + textWidth)) / 2;
    int16_t textX = iconX + iconWidth + ICON_GAP;
    int16_t sunriseY = (gfx.height() - (sunrise.height + ROW_GAP + sunset.height)) / 2;
    int16_t sunsetY = sunriseY + sunrise.height + ROW_GAP;

    gfx.setFont(nullptr); // Top-left anchored
    gfx.setTextSize(1);
    gfx.setTextColor(display.color(255, 255, 255));

    display.drawBitmap(sunrise, iconX, sunriseY);
    gfx.setCursor(textX, sunriseY + (sunrise.height - GLYPH_HEIGHT) / 2);
    gfx.print(data.sunrise);

    display.drawBitmap(sunset, iconX, sunsetY);
    gfx.setCursor(textX, sunsetY + (sunset.height - GLYPH_HEIGHT) / 2);
    gfx.print(data.sunset);

    if (weather.isStale())
    {
        display.drawStaleMarker();
    }
}
