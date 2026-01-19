#include "WeatherManager.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "managers/DisplayManager.h"

WeatherManager::WeatherManager() : config(ConfigManager::getInstance())
{
}

void WeatherManager::begin()
{
    syncWeather();
    syncAqi();
    lastSync = millis();
}

void WeatherManager::update()
{
    if (millis() - lastSync > syncInterval)
    {
        syncWeather();
        syncAqi();
        setDirty(true);
        lastSync = millis();
    }
}

void WeatherManager::syncWeather()
{
    Serial.println("Syncing weather...");
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    String url = buildWeatherURL();
    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error)
        {
            currentTemperature = doc["current"]["temperature_2m"];
            currentWeatherCode = doc["current"]["weather_code"];
            currentIsDay = doc["current"]["is_day"];
            JsonArray weatherCodes = doc["daily"]["weather_code"].as<JsonArray>();
            for (size_t i = 0; i < weatherCodes.size(); i++)
            {
                daysWeatherCode[i] = weatherCodes[i].as<int>();
            }
            JsonArray precipitationSums = doc["daily"]["precipitation_sum"].as<JsonArray>();
            for (size_t i = 0; i < precipitationSums.size(); i++)
            {
                daysPrecipitationSum[i] = precipitationSums[i].as<float>();
            }
            JsonArray maxTemperatures = doc["daily"]["temperature_2m_max"].as<JsonArray>();
            for (size_t i = 0; i < maxTemperatures.size(); i++)
            {
                daysMaxTemperature[i] = maxTemperatures[i].as<float>();
            }
            JsonArray minTemperatures = doc["daily"]["temperature_2m_min"].as<JsonArray>();
            for (size_t i = 0; i < minTemperatures.size(); i++)
            {
                daysMinTemperature[i] = minTemperatures[i].as<float>();
            }
            http.end();
            Serial.println("Weather sync successful!");
        }
        else
        {
            Serial.println("Error processing response");
        }
    }
    else
    {
        Serial.println("Error fetching weather: " + String(httpCode));
    }
    http.end();
}

void WeatherManager::syncAqi()
{
    Serial.println("Syncing AQI...");
    HTTPClient http;
    String url = buildAqiURL();
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error)
        {
            currentAqi = doc["current"]["european_aqi"];
            http.end();
        }
        else
        {
            Serial.println("Error processing response");
        }
    }
    else
    {
        Serial.println("Error fetching AQI: " + String(httpCode));
    }

    http.end();
    Serial.println("AQI sync successful!");
}

String WeatherManager::buildWeatherURL()
{
    String url = WEATHER_BASE_URL;
    url += "?latitude=" + String(config.latitude, 4);
    url += "&longitude=" + String(config.longitude, 4);
    url += "&daily=weather_code,precipitation_sum,temperature_2m_max,temperature_2m_min&current=temperature_2m,weather_code,is_day&timezone=Europe%2FBerlin&forecast_days=2";
    return url;
}

String WeatherManager::buildAqiURL()
{
    String url = AQI_BASE_URL;
    url += "?latitude=" + String(config.latitude, 4);
    url += "&longitude=" + String(config.longitude, 4);
    url += "&current=european_aqi&timezone=Europe%2FBerlin";
    return url;
}

const uint16_t *WeatherManager::getBitmap(int weatherCode, bool checkDay)
{
    switch (weatherCode)
    {
    case 0:
        return currentIsDay == 0 && checkDay ? bitmap_clear_2 : bitmap_clear_1;
    case 1:
    case 2:
        return currentIsDay == 0 && checkDay ? bitmap_cloudy_2 : bitmap_cloudy_1;
    case 3:
    case 45:
    case 48:
        return bitmap_overcast;
    case 51:
    case 61:
    case 80:
        return bitmap_slight_rain;
    case 53:
    case 55:
    case 63:
    case 65:
    case 81:
    case 82:
        return bitmap_heavy_rain;
    case 56:
    case 66:
    case 71:
    case 85:
        return bitmap_slight_snow;
    case 57:
    case 67:
    case 73:
    case 75:
    case 77:
    case 86:
        return bitmap_heavy_snow;
    case 95:
    case 96:
    case 99:
        return bitmap_thunder;
    default:
        Serial.print("Unknown weather code: ");
        Serial.println(weatherCode);
        return bitmap_clear_1;
    }
}

const uint16_t WeatherManager::getAqiColor(int aqi)
{
    if (aqi < 20)
        return displayManager.color565(0, 255, 0); // Good
    else if (aqi < 40)
        return displayManager.color565(255, 255, 0); // Fair
    else if (aqi < 60)
        return displayManager.color565(255, 128, 0); // Moderate
    else if (aqi < 80)
        return displayManager.color565(255, 0, 0); // Poor
    else if (aqi < 100)
        return displayManager.color565(255, 0, 255); // Very poor
    else
        return displayManager.color565(0, 0, 255); // Extremely poor
}
