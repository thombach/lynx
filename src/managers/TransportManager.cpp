#include "TransportManager.h"
#include "ConfigManager.h"
#include "net/Http.h"

namespace
{
    constexpr const char *DEPARTURES_URL = "https://data.grandlyon.com/fr/datapusher/ws/rdata/tcl_sytral.tclpassagearret/all.json";
    constexpr const char *ALERTS_URL = "https://data.grandlyon.com/fr/datapusher/ws/rdata/tcl_sytral.tclalertetrafic_2/all.json";

    time_t parseLocalTime(const char *text)
    {
        struct tm t = {};
        if (sscanf(text, "%d-%d-%d %d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday,
                   &t.tm_hour, &t.tm_min, &t.tm_sec) != 6)
        {
            return 0;
        }
        t.tm_year -= 1900;
        t.tm_mon -= 1;
        t.tm_isdst = -1;
        return mktime(&t);
    }

    void insertSorted(StopDepartures &stop, JsonObjectConst obj)
    {
        const char *time = obj["heurepassage"];
        size_t pos = 0;
        while (pos < stop.count && strcmp(stop.next[pos].time.c_str(), time) <= 0)
        {
            pos++;
        }
        if (pos >= MAX_DEPARTURES)
        {
            return;
        }

        for (size_t i = min(stop.count, MAX_DEPARTURES - 1); i > pos; i--)
        {
            stop.next[i] = stop.next[i - 1];
        }
        Departure &departure = stop.next[pos];
        departure.time = time;
        departure.at = parseLocalTime(time);
        departure.label = obj["delaipassage"] | "";
        stop.count = min(stop.count + 1, MAX_DEPARTURES);

        if (stop.direction.isEmpty())
        {
            stop.direction = obj["direction"] | "";
        }
    }
}

bool TransportManager::fetch()
{
    TransportData data;
    if (!fetchDepartures(data))
    {
        return false;
    }

    // Alerts are secondary: keep the last known state if they can't be fetched
    bool disrupted;
    data.disrupted = fetchDisrupted(disrupted) ? disrupted : get().disrupted;

    publish(data);
    return true;
}

bool TransportManager::fetchDepartures(TransportData &data)
{
    const ConfigManager &config = ConfigManager::getInstance();
    const int stopIds[2] = {config.apiTransportStopId1, config.apiTransportStopId2};

    String url = DEPARTURES_URL;
    url += "?id__in=" + String(stopIds[0]);
    if (stopIds[1] > 0)
    {
        url += "," + String(stopIds[1]);
    }

    JsonDocument filter;
    JsonObject item = filter["values"].add<JsonObject>();
    item["id"] = true;
    item["heurepassage"] = true;
    item["delaipassage"] = true;
    item["direction"] = true;

    JsonDocument doc;
    if (!Http::getJson(url, doc, filter, config.apiTransportUsername.c_str(), config.apiTransportPassword.c_str()))
    {
        return false;
    }

    for (JsonObjectConst obj : doc["values"].as<JsonArrayConst>())
    {
        int id = obj["id"] | 0;
        if (!obj["heurepassage"].is<const char *>())
        {
            continue;
        }
        for (size_t i = 0; i < 2; i++)
        {
            if (stopIds[i] > 0 && id == stopIds[i])
            {
                insertSorted(data.stops[i], obj);
            }
        }
    }
    return true;
}

bool TransportManager::fetchDisrupted(bool &disrupted)
{
    const ConfigManager &config = ConfigManager::getInstance();
    String url = String(ALERTS_URL) + "?ligne_cli__eq=" + config.apiTransportLigneCli;

    JsonDocument filter;
    filter["values"].add<JsonObject>()["ligne_cli"] = true;

    JsonDocument doc;
    if (!Http::getJson(url, doc, filter, config.apiTransportUsername.c_str(), config.apiTransportPassword.c_str()))
    {
        return false;
    }
    disrupted = doc["values"].size() > 0;
    return true;
}
