#pragma once

#include <Adafruit_Protomatter.h>
#include "../managers/WeatherManager.h"
#include "../managers/TimeManager.h"
#include "Screen.h"

class ForecastWeatherScreen : public Screen
{
public:
    ForecastWeatherScreen(Adafruit_Protomatter *matrix, WeatherManager *weatherManager, TimeManager *timeManager);
    void draw() override;
    void update() override;

private:
    Adafruit_Protomatter *matrix;
    WeatherManager *weatherManager;
    TimeManager *timeManager;
};
