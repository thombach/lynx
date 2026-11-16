#include "TransportScreen.h"
#include <Fonts/Picopixel.h>
#include "core/TimeMath.h"
#include "managers/ConfigManager.h"
#include "ui/Text.h"

namespace
{
    constexpr int16_t ROW_HEIGHT = 17;
    constexpr int16_t BADGE_SIZE = 11;
    constexpr uint32_t DEPARTURES_VIEW_MS = 8000;
    constexpr uint32_t ALERTS_VIEW_MS = 12000;

    // Countdown from the scheduled time so it stays accurate between syncs
    String formatDeparture(const Departure &departure, time_t now)
    {
        if (!departure.at || !now)
        {
            return departure.label;
        }
        long minutes = core::minutesUntil(departure.at, now);
        if (minutes < 0)
        {
            return "";
        }
        return minutes == 0 ? String("Proche") : String(minutes) + " min";
    }
}

void TransportScreen::onShow()
{
    shownAt = millis();
    showingAlert = false;
    direction1.reset();
    direction2.reset();
}

void TransportScreen::draw()
{
    if (!transport.hasData())
    {
        display.drawMessage("Loading...", "transports");
        return;
    }

    TransportData data = transport.get();
    bool alertView = data.disrupted() &&
                     (millis() - shownAt) % (DEPARTURES_VIEW_MS + ALERTS_VIEW_MS) >= DEPARTURES_VIEW_MS;
    if (alertView != showingAlert)
    {
        showingAlert = alertView;
        alertText.reset();
        direction1.reset();
        direction2.reset();
    }

    if (alertView)
    {
        drawAlerts(data.alerts);
    }
    else
    {
        drawDepartures(data);
    }

    if (transport.isStale())
    {
        display.drawStaleMarker();
    }
}

void TransportScreen::drawDepartures(const TransportData &data)
{
    time_t now = time.epoch();
    drawStop(data.stops[0], 0, data.disrupted(), now);
    if (ConfigManager::getInstance().apiTransportStopId2 > 0)
    {
        display.gfx().drawFastHLine(0, 16, display.gfx().width(), display.color(30, 30, 30));
        drawStop(data.stops[1], 1, data.disrupted(), now);
    }
}

void TransportScreen::drawStop(const StopDepartures &stop, int index, bool disrupted, time_t now)
{
    Adafruit_Protomatter &gfx = display.gfx();
    int16_t top = index * ROW_HEIGHT;
    uint16_t white = display.color(255, 255, 255);

    drawLineBadge(2, top + 2);

    String direction = toAscii(stop.direction);
    if (disrupted)
    {
        direction = "! " + direction;
    }
    ScrollText &scroll = index == 0 ? direction1 : direction2;
    scroll.draw(display, direction, 15, top + 1, disrupted ? display.color(255, 0, 0) : white);

    String times;
    int shown = 0;
    for (size_t i = 0; i < stop.count && shown < 2; i++)
    {
        String label = formatDeparture(stop.next[i], now);
        if (label.isEmpty())
        {
            continue;
        }
        times += shown++ ? ", " + label : label;
    }

    gfx.setFont(&Picopixel);
    gfx.setTextSize(1);
    gfx.setTextColor(white);
    gfx.setCursor(15, top + 12);
    gfx.print(times.isEmpty() ? "--" : times);
}

void TransportScreen::drawAlerts(const String &alerts)
{
    Adafruit_Protomatter &gfx = display.gfx();
    drawLineBadge(2, 2);

    gfx.setFont(&Picopixel);
    gfx.setTextSize(1);
    gfx.setTextColor(display.color(255, 0, 0));
    gfx.setCursor(16, 9);
    gfx.print("Info trafic");

    alertText.draw(display, toAscii(alerts), 0, 19, display.color(255, 120, 0));
}

// Rounded badge in the line's color with its name
void TransportScreen::drawLineBadge(int16_t x, int16_t y)
{
    static const struct
    {
        const char *prefix;
        uint8_t r, g, b;
    } LINE_COLORS[] = {
        {"A", 248, 68, 144},
        {"B", 0, 132, 207},
        {"C", 242, 145, 0},
        {"D", 0, 164, 82},
        {"F", 132, 189, 0},
        {"T", 139, 39, 133},
    };

    Adafruit_Protomatter &gfx = display.gfx();
    const String &line = ConfigManager::getInstance().apiTransportLigneCli;

    uint16_t background = display.color(90, 90, 90);
    for (const auto &entry : LINE_COLORS)
    {
        if (line.startsWith(entry.prefix))
        {
            background = display.color(entry.r, entry.g, entry.b);
            break;
        }
    }
    gfx.fillRoundRect(x, y, BADGE_SIZE, BADGE_SIZE, 2, background);
    gfx.setTextColor(0); // Black: unlit LEDs read best on a colored badge

    if (line.length() == 1)
    {
        gfx.setFont(nullptr); // Classic 5x7 font, top-left anchored
        gfx.setTextSize(1);
        gfx.setCursor(x + 3, y + 2);
        gfx.print(line);
        return;
    }

    int16_t x1, y1;
    uint16_t w, h;
    gfx.setFont(&Picopixel);
    gfx.getTextBounds(line, 0, 0, &x1, &y1, &w, &h);
    gfx.setCursor(x + (BADGE_SIZE - w) / 2 - x1, y + 8);
    gfx.print(line);
}
