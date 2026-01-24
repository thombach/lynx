#include "ConfigManager.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

ConfigManager &ConfigManager::getInstance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::load()
{
    Serial.println("Loading config file...");
    if (!LittleFS.begin(true, "/littlefs", 10, "ffat"))
    {
        Serial.println("Failed to mount LittleFS");
        return false;
    }

    File file = LittleFS.open("/config.json", "r");
    if (!file)
    {
        Serial.println("Could not open config.json");
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, file);

    if (err)
    {
        Serial.println("Failed to parse JSON");
        return false;
    }

    wifiSsid = doc["wifi_ssid"].as<String>();
    wifiPassword = doc["wifi_password"].as<String>();
    brightness = doc["brightness"].as<float>();
    screenInterval = doc["screen_interval"].as<int>();
    latitude = doc["latitude"].as<float>();
    longitude = doc["longitude"].as<float>();
    gmtOffset = doc["gmt_offset"].as<int>();
    dstOffset = doc["dst_offset"].as<int>();
    apiBikeStationId = doc["api_bike_station_id"].as<int>();
    apiTransportStopId1 = doc["api_transport_stop_id_1"].as<int>();
    apiTransportStopId2 = doc["api_transport_stop_id_2"].as<int>();
    apiTransportLigneCli = doc["api_transport_ligne_cli"].as<String>();
    apiTransportUsername = doc["api_transport_username"].as<String>();
    apiTransportPassword = doc["api_transport_password"].as<String>();

    file.close();
    Serial.println("Config loaded !");
    return true;
}

void ConfigManager::increaseBrightness()
{
    brightness += 0.05;

    if (brightness > 1.0)
    {
        brightness = 0.05;
    }
}