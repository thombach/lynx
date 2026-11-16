#pragma once
#include "Screen.h"
#include "managers/DisplayManager.h"
#include "managers/WeatherManager.h"

// Today's sunrise and sunset
class SunScreen : public Screen
{
public:
    SunScreen(DisplayManager &display, WeatherManager &weather) : display(display), weather(weather) {}
    void draw() override;

private:
    DisplayManager &display;
    WeatherManager &weather;
};
