#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ConfigManager.h"
#include "Manager.cpp"

class TransportManager : public Manager
{
public:
    String line1;
    String direction1;
    String next1[2];
    String line2;
    String direction2;
    String next2[2];
    bool isPerturbations;

    TransportManager();
    void begin();
    void update();

private:
    ConfigManager &config;
    static constexpr const char *TIME_BASE_URL = "https://data.grandlyon.com/fr/datapusher/ws/rdata/tcl_sytral.tclpassagearret/all.json";
    static constexpr const char *PERTURBATIONS_BASE_URL = "https://data.grandlyon.com/fr/datapusher/ws/rdata/tcl_sytral.tclalertetrafic_2/all.json";
    unsigned long lastSync = 0;
    const unsigned long syncInterval = 60UL * 1000UL; // every min
    void syncTime();
    void syncPerturbations();
    String buildTimeURL();
    String buildPerturbationsURL();
    void processResponse(JsonArray values);
};