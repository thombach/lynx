#pragma once
#include <Arduino.h>
#include "bitmap.h"
#include "ConfigManager.h"
#include "Manager.cpp"

class WeatherManager : public Manager
{
public:
    float currentTemperature;
    int currentWeatherCode;
    int currentIsDay;
    int currentAqi;
    int daysWeatherCode[14];
    float daysPrecipitationSum[14];
    float daysMaxTemperature[14];
    float daysMinTemperature[14];

    WeatherManager();
    const uint16_t *getBitmap(int weatherCode, bool checkDay);
    const uint16_t getAqiColor(int aqi);
    void begin();
    void update();

private:
    ConfigManager &config;
    static constexpr const char *WEATHER_BASE_URL = "https://api.open-meteo.com/v1/forecast";
    static constexpr const char *AQI_BASE_URL = "https://air-quality-api.open-meteo.com/v1/air-quality";
    unsigned long lastSync = 0;
    const unsigned long syncInterval = 15UL * 60UL * 1000UL; // every 15min
    void syncWeather();
    void syncAqi();
    String buildWeatherURL();
    String buildAqiURL();
};