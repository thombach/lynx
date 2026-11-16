#include "Http.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "RootCAs.h"

namespace
{
    constexpr int32_t CONNECT_TIMEOUT_MS = 5000;
    constexpr uint16_t READ_TIMEOUT_MS = 8000;

    String error;

    bool fail(const String &message)
    {
        error = message;
        Serial.printf("[http] %s\n", message.c_str());
        return false;
    }
}

const String &Http::lastError()
{
    return error;
}

bool Http::getJson(const String &url, JsonDocument &doc, const JsonDocument &filter,
                   const char *username, const char *password)
{
    error = "";
    WiFiClientSecure client;
    client.setCACert(ROOT_CA_CERTS);

    HTTPClient http;
    http.useHTTP10(true); // Avoids chunked encoding so the raw stream is valid JSON
    http.setConnectTimeout(CONNECT_TIMEOUT_MS);
    http.setTimeout(READ_TIMEOUT_MS);

    if (!http.begin(client, url))
    {
        return fail("Invalid URL: " + url);
    }
    if (username && *username)
    {
        http.setAuthorization(username, password);
    }

    int code = http.GET();
    if (code != HTTP_CODE_OK)
    {
        http.end();
        return fail("GET " + url + " failed: " + (code < 0 ? HTTPClient::errorToString(code) : "HTTP " + String(code)));
    }

    DeserializationError parseError = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
    http.end();
    if (parseError)
    {
        return fail("Invalid JSON from " + url + ": " + parseError.c_str());
    }
    return true;
}
