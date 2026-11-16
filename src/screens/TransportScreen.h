#pragma once
#include "Screen.h"
#include "managers/DisplayManager.h"
#include "managers/TimeManager.h"
#include "managers/TransportManager.h"
#include "ui/ScrollText.h"

// Next departures; while the line has alerts, alternates with a scrolling alert view
class TransportScreen : public Screen
{
public:
    TransportScreen(DisplayManager &display, TransportManager &transport, TimeManager &time)
        : display(display), transport(transport), time(time) {}
    void onShow() override;
    void draw() override;

private:
    DisplayManager &display;
    TransportManager &transport;
    TimeManager &time;
    ScrollText direction1{48};
    ScrollText direction2{48};
    ScrollText alertText{64, ScrollText::Font::Large, ScrollText::Mode::Marquee};
    uint32_t shownAt = 0;
    bool showingAlert = false;

    void drawDepartures(const TransportData &data);
    void drawStop(const StopDepartures &stop, int index, bool disrupted, time_t now);
    void drawAlerts(const String &alerts);
    void drawLineBadge(int16_t x, int16_t y);
};
