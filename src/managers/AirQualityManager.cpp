#include "AirQualityManager.h"
#include "ConfigManager.h"
#include "net/Http.h"

bool AirQualityManager::fetch()
{
    const ConfigManager &config = ConfigManager::getInstance();
    String url = "https://air-quality-api.open-meteo.com/v1/air-quality";
    url += "?latitude=" + String(config.latitude, 4);
    url += "&longitude=" + String(config.longitude, 4);
    url += "&current=european_aqi&timezone=auto";

    JsonDocument filter;
    filter["current"]["european_aqi"] = true;

    JsonDocument doc;
    if (!Http::getJson(url, doc, filter))
    {
        return false;
    }

    JsonVariantConst aqi = doc["current"]["european_aqi"];
    if (!aqi.is<int>())
    {
        return fail("Unexpected response");
    }

    AirQualityData data;
    data.aqi = aqi;
    publish(data);
    return true;
}
