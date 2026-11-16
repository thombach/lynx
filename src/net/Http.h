#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

namespace Http
{
    // HTTPS GET with certificate verification; the body is parsed while streaming,
    // keeping only the fields present in `filter`.
    bool getJson(const String &url, JsonDocument &doc, const JsonDocument &filter,
                 const char *username = nullptr, const char *password = nullptr);

    // Reason of the last failed getJson() call. Not synchronized: only the fetch task makes requests
    const String &lastError();
}
