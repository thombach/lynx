#pragma once
#include "Screen.h"
#include "managers/DisplayManager.h"
#include "managers/TimeManager.h"
#include "managers/WeatherManager.h"

class ForecastWeatherScreen : public Screen
{
public:
    ForecastWeatherScreen(DisplayManager &display, WeatherManager &weather, TimeManager &time)
        : display(display), weather(weather), time(time) {}
    void draw() override;

private:
    DisplayManager &display;
    WeatherManager &weather;
    TimeManager &time;

    void drawTemperature(float value, int16_t x, int16_t y, uint16_t color);
};
