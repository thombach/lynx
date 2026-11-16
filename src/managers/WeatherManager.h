#pragma once
#include "Manager.h"

constexpr size_t HOURLY_POINTS = 12;

struct DailyForecast
{
    int weatherCode = 0;
    float maxTemperature = 0;
    float minTemperature = 0;
};

struct WeatherData
{
    float temperature = 0;
    int weatherCode = 0;
    bool isDay = true;
    DailyForecast days[2]; // Tomorrow, the day after

    float hourlyTemperature[HOURLY_POINTS] = {};   // From the current hour on
    float hourlyPrecipitation[HOURLY_POINTS] = {}; // mm over the preceding hour
    int firstHour = 0;                       // Hour of day of hourlyTemperature[0]

    char sunrise[6] = ""; // "HH:MM", today
    char sunset[6] = "";
};

class WeatherManager : public DataManager<WeatherData>
{
public:
    WeatherManager() : DataManager("weather", 15UL * 60 * 1000) {}

protected:
    bool fetch() override;
};
