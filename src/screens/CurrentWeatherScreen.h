#pragma once

#include <Adafruit_Protomatter.h>
#include "../managers/WeatherManager.h"
#include "Screen.h"

class CurrentWeatherScreen : public Screen
{
public:
    CurrentWeatherScreen(Adafruit_Protomatter *matrix, WeatherManager *weatherManager);
    void draw() override;
    void update() override;

private:
    Adafruit_Protomatter *matrix;
    WeatherManager *weatherManager;
};
