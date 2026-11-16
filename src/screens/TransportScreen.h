#pragma once
#include "Screen.h"
#include "managers/DisplayManager.h"
#include "managers/TimeManager.h"
#include "managers/TransportManager.h"
#include "ui/ScrollText.h"

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

    void drawLineBadge(int16_t x, int16_t y);
    void drawStop(const StopDepartures &stop, int index, bool disrupted, time_t now);
};
