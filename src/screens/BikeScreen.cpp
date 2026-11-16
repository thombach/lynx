#include "BikeScreen.h"
#include <Fonts/Picopixel.h>
#include "bitmap.h"
#include "ui/Text.h"

namespace
{
    constexpr int16_t ICON_SLOT = 5;
}

void BikeScreen::draw()
{
    if (!bikes.hasData())
    {
        display.drawMessage("Loading...", "bikes");
        return;
    }

    BikeData data = bikes.get();
    stationName.draw(display, toAscii(data.stationName), 2, 1, display.color(255, 255, 255));

    display.drawBitmap(bitmap_bike, 3, 11);

    display.gfx().setFont(&Picopixel);
    display.gfx().setTextSize(1);
    drawCount(bitmap_heart, data.bikes, 11);
    drawCount(bitmap_lightning, data.eBikes, 18);
    drawCount(bitmap_parking, data.availableStands, 25);

    if (bikes.isStale())
    {
        display.drawStaleMarker();
    }
}

// 5x5 icon with the value right-aligned just before it
void BikeScreen::drawCount(const Bitmap &icon, int value, int16_t y)
{
    Adafruit_Protomatter &gfx = display.gfx();
    String text(value);
    int16_t x1, y1;
    uint16_t w, h;

    display.drawBitmapCentered(icon, 56, y, ICON_SLOT, ICON_SLOT);
    gfx.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    gfx.setCursor(54 - w, y + 4);
    gfx.setTextColor(display.color(255, 255, 255));
    gfx.print(text);
}
