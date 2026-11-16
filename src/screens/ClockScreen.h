#pragma once
#include "Screen.h"
#include "managers/DisplayManager.h"
#include "managers/TimeManager.h"

class ClockScreen : public Screen
{
public:
    ClockScreen(DisplayManager &display, TimeManager &time) : display(display), time(time) {}
    void draw() override;

private:
    DisplayManager &display;
    TimeManager &time;
};
