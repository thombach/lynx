#pragma once
#include "Screen.h"
#include "managers/AirQualityManager.h"
#include "managers/DisplayManager.h"
#include "managers/WeatherManager.h"

class CurrentWeatherScreen : public Screen
{
public:
    CurrentWeatherScreen(DisplayManager &display, WeatherManager &weather, AirQualityManager &airQuality)
        : display(display), weather(weather), airQuality(airQuality) {}
    void draw() override;

private:
    DisplayManager &display;
    WeatherManager &weather;
    AirQualityManager &airQuality;
};
