#include "ClockScreen.h"
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/Picopixel.h>

void ClockScreen::draw()
{
    struct tm now;
    if (!time.now(now))
    {
        display.drawMessage("Syncing", "time");
        return;
    }

    Adafruit_Protomatter &gfx = display.gfx();
    int16_t x1, y1;
    uint16_t w, h;

    // Colon blinks with the actual seconds
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), now.tm_sec % 2 == 0 ? "%02d:%02d" : "%02d %02d", now.tm_hour, now.tm_min);
    gfx.setFont(&FreeSans12pt7b);
    gfx.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
    gfx.setCursor((gfx.width() - w) / 2 - x1, h + 2);
    gfx.setTextColor(display.color(255, 255, 255));
    gfx.print(timeStr);

    char dateStr[16];
    snprintf(dateStr, sizeof(dateStr), "%s %d %s %d", TimeManager::weekdayName(now.tm_wday), now.tm_mday,
             TimeManager::monthName(now.tm_mon), now.tm_year + 1900);
    gfx.setFont(&Picopixel);
    gfx.getTextBounds(dateStr, 0, 0, &x1, &y1, &w, &h);
    gfx.setCursor((gfx.width() - w) / 2 - x1, gfx.height() - 4);
    gfx.setTextColor(display.color(255, 128, 0));
    gfx.print(dateStr);
}
