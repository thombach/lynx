#pragma once
#include "Manager.h"

struct AirQualityData
{
    int aqi = 0; // European AQI
};

class AirQualityManager : public DataManager<AirQualityData>
{
public:
    AirQualityManager() : DataManager("aqi", 30UL * 60 * 1000) {}

protected:
    bool fetch() override;
};
