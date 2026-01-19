#include "TransportManager.h"
#include <HTTPClient.h>

TransportManager::TransportManager() : config(ConfigManager::getInstance())
{
}

void TransportManager::begin()
{
    syncTime();
    syncPerturbations();
    lastSync = millis();
}

void TransportManager::update()
{
    if (millis() - lastSync > syncInterval)
    {
        syncTime();
        syncPerturbations();
        setDirty(true);
        lastSync = millis();
    }
}

void TransportManager::syncTime()
{
    Serial.println("Syncing transports...");
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    String url = buildTimeURL();
    http.begin(client, url);
    http.setAuthorization(config.apiTransportUsername.c_str(), config.apiTransportPassword.c_str());
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error)
        {
            processResponse(doc["values"]);
            http.end();
            Serial.println("Transports sync successful!");
        }
        else
        {
            Serial.println("Error processing response");
        }
    }
    else
    {
        Serial.println("Error fetching transports: " + String(httpCode));
        Serial.println(http.getString());
    }
    http.end();
}

void TransportManager::syncPerturbations()
{
    Serial.println("Syncing perturbations...");
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    String url = buildPerturbationsURL();
    http.begin(client, url);
    http.setAuthorization(config.apiTransportUsername.c_str(), config.apiTransportPassword.c_str());
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error)
        {
            isPerturbations = doc["values"].as<JsonArray>().size() > 0;
            http.end();
            Serial.println("Perturbations sync successful!");
        }
        else
        {
            Serial.println("Error processing response");
        }
    }
    else
    {
        Serial.println("Error fetching perturbations: " + String(httpCode));
        Serial.println(http.getString());
    }
    http.end();
}

String TransportManager::buildTimeURL()
{
    String url = TIME_BASE_URL;
    url += "?id__in=" + String(config.apiTransportStopId1) + "," + String(config.apiTransportStopId2);
    return url;
}

String TransportManager::buildPerturbationsURL()
{
    String url = PERTURBATIONS_BASE_URL;
    url += "?ligne_cli__eq=" + String(config.apiTransportLigneCli);
    return url;
}

void TransportManager::processResponse(JsonArray values)
{
    JsonObject first1 = JsonObject();
    JsonObject second1 = JsonObject();
    JsonObject first2 = JsonObject();
    JsonObject second2 = JsonObject();

    for (JsonObject obj : values)
    {
        const char *time = obj["heurepassage"];
        int stop_id = obj["id"];

        if (stop_id == config.apiTransportStopId1)
        {
            if (!first1 || strcmp(time, first1["heurepassage"]) < 0)
            {
                second1 = first1;
                first1 = obj;
            }
            else if (!second1 || strcmp(time, second1["heurepassage"]) < 0)
            {
                second1 = obj;
            }
        }
        if (stop_id == config.apiTransportStopId2)
        {
            if (!first2 || strcmp(time, first2["heurepassage"]) < 0)
            {
                second2 = first2;
                first2 = obj;
            }
            else if (!second2 || strcmp(time, second2["heurepassage"]) < 0)
            {
                second2 = obj;
            }
        }
    }
    line1 = first1["ligne"] ? first1["ligne"].as<String>() : "";
    direction1 = first1["direction"] ? first1["direction"].as<String>() : "";
    next1[0] = first1["delaipassage"] ? first1["delaipassage"].as<String>() : "";
    next1[1] = second1["delaipassage"] ? second1["delaipassage"].as<String>() : "";
    line2 = first2["ligne"] ? first2["ligne"].as<String>() : "";
    direction2 = first2["direction"] ? first2["direction"].as<String>() : "";
    next2[0] = first2["delaipassage"] ? first2["delaipassage"].as<String>() : "";
    next2[1] = second2["delaipassage"] ? second2["delaipassage"].as<String>() : "";
}