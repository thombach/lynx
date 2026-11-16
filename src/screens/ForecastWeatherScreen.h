#pragma once
#include "Screen.h"
#include "managers/DisplayManager.h"
#include "managers/TimeManager.h"
#include "managers/WeatherManager.h"

// Tomorrow and the day after, side by side
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

    int16_t textWidth(const String &text);
    int16_t temperatureWidth(float value);
    void drawTemperature(float value, int16_t x, int16_t baseline);
};
