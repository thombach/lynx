#include "BikeManager.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

BikeManager::BikeManager() : config(ConfigManager::getInstance())
{
}

void BikeManager::begin()
{
    sync();
    lastSync = millis();
}

void BikeManager::update()
{
    if (millis() - lastSync > syncInterval)
    {
        sync();
        setDirty(true);
        lastSync = millis();
    }
}

void BikeManager::sync()
{
    Serial.println("Syncing bikes...");
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    String url = buildURL();
    http.begin(client, url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error)
        {
            bikes = doc["values"][0]["total_stands"]["availabilities"]["mechanicalBikes"];
            eBikes = doc["values"][0]["total_stands"]["availabilities"]["electricalBikes"];
            availableStands = doc["values"][0]["available_bike_stands"];
            stationName = doc["values"][0]["name"].as<String>();
            int dashIndex = stationName.indexOf('-');
            if (dashIndex != -1)
            {
                stationName = stationName.substring(dashIndex + 1);
                stationName.trim();
            }
            http.end();
            Serial.println("Bikes sync successful!");
        }
        else
        {
            Serial.println("Error processing response");
        }
    }
    else
    {
        Serial.println("Error fetching bikes: " + String(httpCode));
        Serial.println(http.getString());
    }
    http.end();
}

String BikeManager::buildURL()
{
    String url = BASE_URL;
    url += "?field=number&value=" + String(config.apiBikeStationId);
    return url;
}