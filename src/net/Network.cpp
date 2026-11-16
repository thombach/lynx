#include "Network.h"
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFi.h>
#include "WebPage.h"
#include "managers/ConfigManager.h"

namespace
{
    constexpr uint32_t CONNECT_TIMEOUT_MS = 30000;
    constexpr uint32_t RESTART_DELAY_MS = 1000;
    constexpr const char *ADMIN_USER = "admin";
    constexpr size_t MAX_MESSAGE_LENGTH = 500;
    constexpr uint32_t DEFAULT_MESSAGE_S = 15;
    constexpr uint32_t MAX_MESSAGE_S = 600;
}

void Network::setFrameSource(const uint16_t *buffer, uint16_t width, uint16_t height)
{
    frame = buffer;
    frameWidth = width;
    frameHeight = height;
}

void Network::begin()
{
    const ConfigManager &config = ConfigManager::getInstance();
    WiFi.persistent(false);
    WiFi.setHostname(HOSTNAME);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    if (!config.hasWifi())
    {
        startPortal();
        return;
    }
    Serial.printf("[wifi] Connecting to %s...\n", config.wifiSsid.c_str());
    WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());
    connectStartedAt = millis();
}

bool Network::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

void Network::loop()
{
    bool connected = isConnected();
    if (connected && !wasConnected)
    {
        wasConnected = true;
        Serial.printf("[wifi] Connected, IP %s\n", WiFi.localIP().toString().c_str());
        if (portalActive)
        {
            stopPortal();
        }
        if (!servicesStarted)
        {
            startServices();
        }
    }

    // Later disconnections are left to auto-reconnect: the portal is for initial setup only
    if (!wasConnected && !portalActive && millis() - connectStartedAt > CONNECT_TIMEOUT_MS)
    {
        startPortal();
    }

    if (servicesStarted)
    {
        ArduinoOTA.handle();
        server.handleClient();
    }
    if (portalActive)
    {
        dns.processNextRequest();
    }
    if (restartAt && (int32_t)(millis() - restartAt) >= 0)
    {
        ESP.restart();
    }
}

void Network::startServices()
{
    servicesStarted = true;

    server.on("/", HTTP_GET, [this]
              { handlePage(); });
    server.on("/config.json", HTTP_GET, [this]
              { handleGetConfig(); });
    server.on("/config", HTTP_POST, [this]
              { handlePostConfig(); });
    server.on("/status.json", HTTP_GET, [this]
              { handleStatus(); });
    server.on("/screen", HTTP_GET, [this]
              { handleScreen(); });
    server.on("/message", HTTP_POST, [this]
              { handleMessage(); });
    server.on("/brightness", HTTP_POST, [this]
              { handleBrightness(); });
    server.on("/update", HTTP_POST, [this]
              { handleFirmwareDone(); }, [this]
              { handleFirmwareUpload(); });
    server.onNotFound([this]
                      { handleNotFound(); });
    server.begin();

    const String &password = ConfigManager::getInstance().adminPassword;
    ArduinoOTA.setHostname(HOSTNAME);
    if (!password.isEmpty())
    {
        ArduinoOTA.setPassword(password.c_str());
    }
    ArduinoOTA.onStart([this]
                       {
        Serial.println("[ota] Update started");
        if (updateStartCallback)
        {
            updateStartCallback();
        } });
    ArduinoOTA.onError([](ota_error_t error)
                       {
        // Fetching was paused for the update: restart to get back to a clean state
        Serial.printf("[ota] Error %u, restarting\n", error);
        ESP.restart(); });
    ArduinoOTA.begin(); // Also announces lynx.local over mDNS
    MDNS.addService("http", "tcp", 80);
}

void Network::startPortal()
{
    Serial.printf("[wifi] Starting setup portal \"%s\"\n", AP_NAME);
    portalActive = true;
    WiFi.mode(WIFI_AP_STA); // The station keeps retrying in the background
    WiFi.softAP(AP_NAME);
    dns.start(53, "*", WiFi.softAPIP());
    if (!servicesStarted)
    {
        startServices();
    }
}

void Network::stopPortal()
{
    Serial.println("[wifi] Closing setup portal");
    dns.stop();
    WiFi.softAPdisconnect(true);
    portalActive = false;
}

void Network::scheduleRestart()
{
    restartAt = millis() + RESTART_DELAY_MS; // Lets the HTTP response reach the browser first
}

bool Network::isAuthorized()
{
    const String &password = ConfigManager::getInstance().adminPassword;
    return password.isEmpty() || server.authenticate(ADMIN_USER, password.c_str());
}

bool Network::authorize()
{
    if (isAuthorized())
    {
        return true;
    }
    server.requestAuthentication();
    return false;
}

void Network::handlePage()
{
    // Also protected so the browser asks for credentials before the page's API calls
    if (authorize())
    {
        server.send_P(200, "text/html", WEB_PAGE);
    }
}

void Network::handleGetConfig()
{
    if (authorize())
    {
        server.send(200, "application/json", ConfigManager::getInstance().toEditableJson());
    }
}

void Network::handlePostConfig()
{
    if (!authorize())
    {
        return;
    }

    String error;
    JsonDocument response;
    if (ConfigManager::getInstance().save(server.arg("plain"), error))
    {
        response["ok"] = true;
        scheduleRestart();
        sendJson(200, response);
    }
    else
    {
        response["error"] = error;
        sendJson(400, response);
    }
}

void Network::sendJson(int code, const JsonDocument &doc)
{
    String body;
    serializeJson(doc, body);
    server.send(code, "application/json", body);
}

void Network::handleStatus()
{
    if (!authorize())
    {
        return;
    }

    JsonDocument doc;
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["connected"] = isConnected();
    wifi["ssid"] = WiFi.SSID();
    wifi["rssi"] = WiFi.RSSI();
    wifi["ip"] = WiFi.localIP().toString();
    wifi["portal"] = portalActive;
    if (statusCallback)
    {
        statusCallback(doc.as<JsonObject>());
    }
    sendJson(200, doc);
}

// Raw little-endian RGB565 pixels of the last frame shown
void Network::handleScreen()
{
    if (!authorize())
    {
        return;
    }
    if (!frame)
    {
        server.send(503, "text/plain", "No frame");
        return;
    }
    server.sendHeader("X-Width", String(frameWidth));
    server.sendHeader("X-Height", String(frameHeight));
    server.sendHeader("Cache-Control", "no-store");
    server.send_P(200, "application/octet-stream", (const char *)frame, frameWidth * frameHeight * sizeof(uint16_t));
}

// Accepts JSON {"text": ..., "duration": seconds, "color": "#rrggbb"}, the same fields
// as form data, or a plain text body
void Network::handleMessage()
{
    if (!authorize())
    {
        return;
    }

    JsonDocument doc;
    if (server.hasArg("text"))
    {
        doc["text"] = server.arg("text");
        long duration = server.arg("duration").toInt();
        if (duration > 0)
        {
            doc["duration"] = duration;
        }
        doc["color"] = server.arg("color");
    }
    else
    {
        String body = server.arg("plain");
        if (deserializeJson(doc, body) || !doc.is<JsonObject>())
        {
            doc.clear();
            doc["text"] = body;
        }
    }

    Message message;
    message.text = doc["text"] | "";
    message.text.trim();
    message.duration = constrain(doc["duration"] | DEFAULT_MESSAGE_S, 1u, MAX_MESSAGE_S) * 1000;
    message.color = {255, 255, 255};
    core::parseHexColor(doc["color"].as<const char *>(), message.color);

    JsonDocument response;
    if (message.text.isEmpty() || message.text.length() > MAX_MESSAGE_LENGTH)
    {
        response["error"] = "text is required, up to " + String(MAX_MESSAGE_LENGTH) + " characters";
        sendJson(400, response);
        return;
    }

    if (messageCallback)
    {
        messageCallback(message);
    }
    response["ok"] = true;
    sendJson(200, response);
}

// Accepts JSON {"brightness": 0.05-1.0} or the same field as form data
void Network::handleBrightness()
{
    if (!authorize())
    {
        return;
    }

    float value = NAN;
    if (server.hasArg("brightness"))
    {
        value = server.arg("brightness").toFloat();
    }
    else
    {
        JsonDocument doc;
        if (!deserializeJson(doc, server.arg("plain")) && doc["brightness"].is<float>())
        {
            value = doc["brightness"].as<float>();
        }
    }

    JsonDocument response;
    // Rounded to 5% steps by the callback: accept anything that rounds into the range
    if (!(value >= 0.025f && value <= 1.025f) || !brightnessCallback)
    {
        response["error"] = "brightness must be between 0.05 and 1.0";
        sendJson(400, response);
        return;
    }
    response["brightness"] = brightnessCallback(value);
    sendJson(200, response);
}

// Called for each chunk of the uploaded file, before handleFirmwareDone()
void Network::handleFirmwareUpload()
{
    HTTPUpload &upload = server.upload();
    switch (upload.status)
    {
    case UPLOAD_FILE_START:
        updateStarted = false;
        updateError = "";
        if (!isAuthorized())
        {
            updateError = "Unauthorized";
            return;
        }
        Serial.printf("[update] Receiving %s\n", upload.filename.c_str());
        updateStarted = true;
        if (updateStartCallback)
        {
            updateStartCallback();
        }
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH))
        {
            updateError = Update.errorString();
        }
        break;

    case UPLOAD_FILE_WRITE:
        if (updateError.isEmpty() && Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            updateError = Update.errorString();
        }
        break;

    case UPLOAD_FILE_END:
        if (updateError.isEmpty() && !Update.end(true))
        {
            updateError = Update.errorString();
        }
        break;

    case UPLOAD_FILE_ABORTED:
        Update.abort();
        updateError = "Upload aborted";
        break;
    }
}

void Network::handleFirmwareDone()
{
    if (!authorize())
    {
        return;
    }

    if (updateError.isEmpty() && updateStarted)
    {
        Serial.println("[update] Installed, restarting");
        server.send(200, "text/plain", "OK");
    }
    else
    {
        Serial.printf("[update] Failed: %s\n", updateError.c_str());
        server.send(500, "text/plain", "Update failed: " + (updateError.isEmpty() ? String("no file received") : updateError));
    }

    // Fetching was paused once the update started: restart either way
    if (updateStarted)
    {
        scheduleRestart();
    }
}

void Network::handleNotFound()
{
    if (portalActive)
    {
        // Captive portal: send every unknown URL to the config page
        server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/", true);
        server.send(302, "text/plain", "");
        return;
    }
    server.send(404, "text/plain", "Not found");
}
