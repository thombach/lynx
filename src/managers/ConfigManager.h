#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

struct ScreenEntry
{
    String name;
    uint32_t duration;
};

class ConfigManager
{
public:
    static ConfigManager &getInstance();

    bool load();
    // Config as editable JSON, with secrets masked
    String toEditableJson() const;
    // Validates and persists a JSON config (applied on next boot); masked secrets keep their value
    bool save(const String &json, String &error);

    bool hasWifi() const { return !wifiSsid.isEmpty(); }
    bool hasWeather() const { return !isnan(latitude) && !isnan(longitude); }
    bool hasBike() const { return apiBikeStationId > 0; }
    bool hasTransport() const;
    bool hasNightMode() const { return nightStart >= 0 && nightEnd >= 0 && nightStart != nightEnd; }

    String wifiSsid;
    String wifiPassword;
    String adminPassword;
    float brightness = 0.5f;
    uint32_t screenInterval = 10000;
    std::vector<ScreenEntry> screens;
    float latitude = NAN;
    float longitude = NAN;
    String timezone;
    int nightStart = -1; // Minutes since midnight, -1 when disabled
    int nightEnd = -1;
    float nightBrightness = 0.0f;
    int apiBikeStationId = 0;
    int apiTransportStopId1 = 0;
    int apiTransportStopId2 = 0;
    String apiTransportLigneCli;
    String apiTransportUsername;
    String apiTransportPassword;

private:
    ConfigManager() = default;
    ConfigManager(const ConfigManager &) = delete;
    void operator=(const ConfigManager &) = delete;

    bool readFile(JsonDocument &doc) const;
    void apply(const JsonDocument &doc);
};
