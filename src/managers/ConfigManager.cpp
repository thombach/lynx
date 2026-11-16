#include "ConfigManager.h"
#include <LittleFS.h>

namespace
{
    constexpr const char *PATH = "/config.json";
    constexpr const char *TMP_PATH = "/config.json.tmp";
    constexpr const char *PARTITION = "ffat";
    constexpr const char *SECRET_MASK = "********";
    constexpr const char *SECRET_KEYS[] = {"wifi_password", "admin_password", "api_transport_password"};
    constexpr const char *DEFAULT_TIMEZONE = "CET-1CEST,M3.5.0,M10.5.0/3"; // Europe/Paris
    constexpr const char *DEFAULT_SCREENS[] = {"clock", "weather", "forecast", "bike", "transport"};
    constexpr uint32_t MIN_SCREEN_DURATION = 1000;

    bool mount(bool formatOnFail)
    {
        return LittleFS.begin(formatOnFail, "/littlefs", 10, PARTITION);
    }

    // "HH:MM" -> minutes since midnight, -1 if missing or invalid
    int parseTime(const char *value)
    {
        int hours, minutes;
        if (!value || sscanf(value, "%d:%d", &hours, &minutes) != 2 ||
            hours < 0 || hours > 23 || minutes < 0 || minutes > 59)
        {
            return -1;
        }
        return hours * 60 + minutes;
    }
}

ConfigManager &ConfigManager::getInstance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::load()
{
    JsonDocument doc;
    bool ok = readFile(doc);
    apply(doc); // Always applied so defaults are set even without a file
    return ok;
}

bool ConfigManager::hasTransport() const
{
    return apiTransportStopId1 > 0 && !apiTransportLigneCli.isEmpty() && !apiTransportUsername.isEmpty();
}

bool ConfigManager::readFile(JsonDocument &doc) const
{
    // No format on failure: an unreadable partition must not be silently wiped
    if (!mount(false))
    {
        Serial.println("[config] Cannot mount filesystem");
        return false;
    }

    File file = LittleFS.open(PATH, "r");
    if (!file)
    {
        Serial.println("[config] config.json not found");
        return false;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error)
    {
        Serial.printf("[config] Invalid config.json: %s\n", error.c_str());
        doc.clear();
        return false;
    }
    return true;
}

void ConfigManager::apply(const JsonDocument &doc)
{
    wifiSsid = doc["wifi_ssid"] | "";
    wifiPassword = doc["wifi_password"] | "";
    adminPassword = doc["admin_password"] | "";
    brightness = constrain(doc["brightness"] | 0.5f, 0.05f, 1.0f);
    screenInterval = max<uint32_t>(doc["screen_interval"] | 10000u, MIN_SCREEN_DURATION);
    latitude = doc["latitude"] | NAN;
    longitude = doc["longitude"] | NAN;
    timezone = doc["timezone"] | DEFAULT_TIMEZONE;
    nightStart = parseTime(doc["night_start"].as<const char *>());
    nightEnd = parseTime(doc["night_end"].as<const char *>());
    nightBrightness = constrain(doc["night_brightness"] | 0.0f, 0.0f, 1.0f);
    apiBikeStationId = doc["api_bike_station_id"] | 0;
    apiTransportStopId1 = doc["api_transport_stop_id_1"] | 0;
    apiTransportStopId2 = doc["api_transport_stop_id_2"] | 0;
    apiTransportLigneCli = doc["api_transport_ligne_cli"] | "";
    apiTransportUsername = doc["api_transport_username"] | "";
    apiTransportPassword = doc["api_transport_password"] | "";

    // Each entry is either "name" or {"name": ..., "duration": ms}
    screens.clear();
    JsonArrayConst list = doc["screens"];
    if (list.isNull())
    {
        for (const char *name : DEFAULT_SCREENS)
        {
            screens.push_back({name, screenInterval});
        }
    }
    for (JsonVariantConst item : list)
    {
        const char *name = item.is<const char *>() ? item.as<const char *>() : item["name"].as<const char *>();
        if (name)
        {
            screens.push_back({name, max<uint32_t>(item["duration"] | screenInterval, MIN_SCREEN_DURATION)});
        }
    }
}

String ConfigManager::toEditableJson() const
{
    JsonDocument doc;
    if (!readFile(doc))
    {
        doc["wifi_ssid"] = "";
        doc["wifi_password"] = "";
        doc["admin_password"] = "";
        doc["latitude"] = 45.7640;
        doc["longitude"] = 4.8357;
    }

    for (const char *key : SECRET_KEYS)
    {
        const char *value = doc[key];
        if (value && *value)
        {
            doc[key] = SECRET_MASK;
        }
    }

    String json;
    serializeJsonPretty(doc, json);
    return json;
}

bool ConfigManager::save(const String &json, String &error)
{
    JsonDocument doc;
    DeserializationError parseError = deserializeJson(doc, json);
    if (parseError)
    {
        error = String("Invalid JSON: ") + parseError.c_str();
        return false;
    }
    if (!doc.is<JsonObject>())
    {
        error = "The config must be a JSON object";
        return false;
    }
    const char *ssid = doc["wifi_ssid"] | "";
    if (!*ssid)
    {
        error = "wifi_ssid is required";
        return false;
    }

    JsonDocument current;
    readFile(current);
    for (const char *key : SECRET_KEYS)
    {
        if (doc[key] == SECRET_MASK)
        {
            doc[key] = current[key];
        }
    }

    // Formatting is acceptable here: the user explicitly provides a full new config
    if (!mount(true))
    {
        error = "Cannot mount filesystem";
        return false;
    }

    // Write then rename, so a power loss never leaves a truncated config
    File file = LittleFS.open(TMP_PATH, "w");
    if (!file || serializeJsonPretty(doc, file) == 0)
    {
        error = "Cannot write config";
        return false;
    }
    file.close();
    if (!LittleFS.rename(TMP_PATH, PATH))
    {
        error = "Cannot replace config";
        return false;
    }
    return true; // Applied after the restart, never while the fetch task reads it
}
