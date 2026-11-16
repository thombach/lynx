#include "WeatherManager.h"
#include "ConfigManager.h"
#include "net/Http.h"

bool WeatherManager::fetch()
{
    const ConfigManager &config = ConfigManager::getInstance();
    String url = "https://api.open-meteo.com/v1/forecast";
    url += "?latitude=" + String(config.latitude, 4);
    url += "&longitude=" + String(config.longitude, 4);
    url += "&current=temperature_2m,weather_code,is_day";
    url += "&daily=weather_code,precipitation_sum,temperature_2m_max,temperature_2m_min";
    url += "&timezone=auto&forecast_days=2";

    JsonDocument filter;
    filter["current"] = true;
    filter["daily"] = true;

    JsonDocument doc;
    if (!Http::getJson(url, doc, filter))
    {
        return false;
    }

    JsonObjectConst current = doc["current"];
    JsonObjectConst daily = doc["daily"];
    if (!current["temperature_2m"].is<float>() || daily["weather_code"].size() < 2)
    {
        Serial.println("[weather] Unexpected response");
        return false;
    }

    WeatherData data;
    data.temperature = current["temperature_2m"];
    data.weatherCode = current["weather_code"];
    data.isDay = current["is_day"] | 1;
    for (size_t i = 0; i < 2; i++)
    {
        data.days[i].weatherCode = daily["weather_code"][i];
        data.days[i].precipitationSum = daily["precipitation_sum"][i];
        data.days[i].maxTemperature = daily["temperature_2m_max"][i];
        data.days[i].minTemperature = daily["temperature_2m_min"][i];
    }
    publish(data);
    return true;
}
