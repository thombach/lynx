#pragma once
#include "Screen.h"
#include "managers/DisplayManager.h"
#include "managers/WeatherManager.h"

// Temperature curve and rain bars for the next 12 hours
class HourlyScreen : public Screen
{
public:
    HourlyScreen(DisplayManager &display, WeatherManager &weather) : display(display), weather(weather) {}
    void draw() override;

private:
    DisplayManager &display;
    WeatherManager &weather;

    int16_t labelWidth(float value);
    void drawTemperatureLabel(float value, int16_t x, int16_t y);
};
