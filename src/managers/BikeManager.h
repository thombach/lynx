#pragma once
#include <Arduino.h>
#include "ConfigManager.h"
#include "Manager.cpp"

class BikeManager : public Manager
{
public:
    int bikes;
    int eBikes;
    int availableStands;
    String stationName;

    BikeManager();
    void begin();
    void update();

private:
    ConfigManager &config;
    static constexpr const char *BASE_URL = "https://data.grandlyon.com/fr/datapusher/ws/rdata/jcd_jcdecaux.jcdvelov/all.json";
    unsigned long lastSync = 0;
    const unsigned long syncInterval = 60UL * 1000UL; // every min
    void sync();
    String buildURL();
};