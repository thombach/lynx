#pragma once
#include "Manager.h"

struct BikeData
{
    String stationName;
    int bikes = 0;
    int eBikes = 0;
    int availableStands = 0;
};

class BikeManager : public DataManager<BikeData>
{
public:
    BikeManager() : DataManager("bike", 60UL * 1000) {}

protected:
    bool fetch() override;
};
