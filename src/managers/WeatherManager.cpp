#include "WeatherManager.h"
#include "ConfigManager.h"

namespace
{
    // "2026-11-16T07:41" -> "07:41"
    void copyClock(const char *isoTime, char (&out)[6])
    {
        const char *t = isoTime ? strchr(isoTime, 'T') : nullptr;
        strlcpy(out, t ? t + 1 : "", sizeof(out));
    }
}

bool WeatherManager::fetch()
{
    const ConfigManager &config = ConfigManager::getInstance();
    String url = "https://api.open-meteo.com/v1/forecast";
    url += "?latitude=" + String(config.latitude, 4);
    url += "&longitude=" + String(config.longitude, 4);
    url += "&current=temperature_2m,weather_code,is_day";
    url += "&hourly=temperature_2m,precipitation&forecast_hours=" + String(HOURLY_POINTS);
    url += "&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset";
    url += "&timezone=auto&forecast_days=3";

    JsonDocument filter;
    filter["current"] = true;
    filter["hourly"] = true;
    filter["daily"] = true;

    JsonDocument doc;
    if (!Http::getJson(url, doc, filter))
    {
        return false;
    }

    JsonObjectConst current = doc["current"];
    JsonObjectConst hourly = doc["hourly"];
    JsonObjectConst daily = doc["daily"];
    if (!current["temperature_2m"].is<float>() || daily["weather_code"].size() < 3 ||
        hourly["temperature_2m"].size() < HOURLY_POINTS)
    {
        return fail("Unexpected response");
    }

    WeatherData data;
    data.temperature = current["temperature_2m"];
    data.weatherCode = current["weather_code"];
    data.isDay = current["is_day"] | 1;

    // Daily index 0 is today, only used for the sun times below
    for (size_t i = 0; i < 2; i++)
    {
        data.days[i].weatherCode = daily["weather_code"][i + 1];
        data.days[i].maxTemperature = daily["temperature_2m_max"][i + 1];
        data.days[i].minTemperature = daily["temperature_2m_min"][i + 1];
    }

    for (size_t i = 0; i < HOURLY_POINTS; i++)
    {
        data.hourlyTemperature[i] = hourly["temperature_2m"][i];
        data.hourlyPrecipitation[i] = hourly["precipitation"][i] | 0.0f;
    }
    char firstHour[6] = "";
    copyClock(hourly["time"][0].as<const char *>(), firstHour);
    data.firstHour = atoi(firstHour);

    copyClock(daily["sunrise"][0].as<const char *>(), data.sunrise);
    copyClock(daily["sunset"][0].as<const char *>(), data.sunset);

    publish(data);
    return true;
}
