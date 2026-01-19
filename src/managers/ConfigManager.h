#pragma once
#include <Arduino.h>

class ConfigManager
{
public:
    static ConfigManager &getInstance();

    bool load();

    String wifiSsid;
    String wifiPassword;
    float brightness;
    int screenInterval;
    float latitude;
    float longitude;
    int gmtOffset;
    int dstOffset;
    int apiBikeStationId;
    int apiTransportStopId1;
    int apiTransportStopId2;
    String apiTransportLigneCli;
    String apiTransportUsername;
    String apiTransportPassword;

private:
    ConfigManager() {}
    ConfigManager(const ConfigManager &) = delete;
    void operator=(const ConfigManager &) = delete;
};