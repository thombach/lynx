#pragma once
#include "Manager.h"

struct DailyForecast
{
    int weatherCode = 0;
    float precipitationSum = 0;
    float maxTemperature = 0;
    float minTemperature = 0;
};

struct WeatherData
{
    float temperature = 0;
    int weatherCode = 0;
    bool isDay = true;
    DailyForecast days[2]; // Today, tomorrow
};

class WeatherManager : public DataManager<WeatherData>
{
public:
    WeatherManager() : DataManager("weather", 15UL * 60 * 1000) {}

protected:
    bool fetch() override;
};
