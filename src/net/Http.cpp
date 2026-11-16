#include "Http.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "RootCAs.h"

namespace
{
    constexpr int32_t CONNECT_TIMEOUT_MS = 5000;
    constexpr uint16_t READ_TIMEOUT_MS = 8000;
}

bool Http::getJson(const String &url, JsonDocument &doc, const JsonDocument &filter,
                   const char *username, const char *password)
{
    WiFiClientSecure client;
    client.setCACert(ROOT_CA_CERTS);

    HTTPClient http;
    http.useHTTP10(true); // Avoids chunked encoding so the raw stream is valid JSON
    http.setConnectTimeout(CONNECT_TIMEOUT_MS);
    http.setTimeout(READ_TIMEOUT_MS);

    if (!http.begin(client, url))
    {
        Serial.printf("[http] Invalid URL: %s\n", url.c_str());
        return false;
    }
    if (username && *username)
    {
        http.setAuthorization(username, password);
    }

    int code = http.GET();
    if (code != HTTP_CODE_OK)
    {
        Serial.printf("[http] GET %s failed: %d %s\n", url.c_str(), code,
                      code < 0 ? HTTPClient::errorToString(code).c_str() : "");
        http.end();
        return false;
    }

    DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
    http.end();
    if (error)
    {
        Serial.printf("[http] Invalid JSON from %s: %s\n", url.c_str(), error.c_str());
        return false;
    }
    return true;
}
