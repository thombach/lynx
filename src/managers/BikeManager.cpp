#include "BikeManager.h"
#include "ConfigManager.h"
#include "net/Http.h"

bool BikeManager::fetch()
{
    String url = "https://data.grandlyon.com/fr/datapusher/ws/rdata/jcd_jcdecaux.jcdvelov/all.json";
    url += "?field=number&value=" + String(ConfigManager::getInstance().apiBikeStationId);

    JsonDocument filter;
    JsonObject station = filter["values"].add<JsonObject>();
    station["name"] = true;
    station["available_bike_stands"] = true;
    station["total_stands"]["availabilities"] = true;

    JsonDocument doc;
    if (!Http::getJson(url, doc, filter))
    {
        return false;
    }

    JsonObjectConst values = doc["values"][0];
    if (values.isNull())
    {
        Serial.println("[bike] Station not found");
        return false;
    }

    BikeData data;
    data.bikes = values["total_stands"]["availabilities"]["mechanicalBikes"] | 0;
    data.eBikes = values["total_stands"]["availabilities"]["electricalBikes"] | 0;
    data.availableStands = values["available_bike_stands"] | 0;

    // "10022 - PRIMAT / DUCROIZE" -> "PRIMAT / DUCROIZE"
    data.stationName = values["name"] | "";
    int dash = data.stationName.indexOf('-');
    if (dash != -1)
    {
        data.stationName = data.stationName.substring(dash + 1);
        data.stationName.trim();
    }

    publish(data);
    return true;
}
