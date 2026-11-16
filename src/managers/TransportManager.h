#pragma once
#include "Manager.h"

struct Departure
{
    String time;   // "YYYY-MM-DD HH:MM:SS", local time
    time_t at = 0; // Parsed `time`, 0 if it couldn't be parsed
    String label;  // Delay computed by the API at fetch time, used as a fallback
};

constexpr size_t MAX_DEPARTURES = 3;

struct StopDepartures
{
    String direction;
    Departure next[MAX_DEPARTURES]; // Sorted, soonest first
    size_t count = 0;
};

struct TransportData
{
    StopDepartures stops[2];
    bool disrupted = false;
};

class TransportManager : public DataManager<TransportData>
{
public:
    TransportManager() : DataManager("transport", 60UL * 1000) {}

protected:
    bool fetch() override;

private:
    bool fetchDepartures(TransportData &data);
    bool fetchDisrupted(bool &disrupted);
};
