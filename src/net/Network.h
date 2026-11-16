#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <functional>
#include "core/Color.h"

struct Message
{
    String text;
    uint32_t duration; // ms
    core::Rgb color;
};

// Wi-Fi connection, setup portal, web page with its API, and OTA updates.
// If the station can't connect, an open access point with a captive portal is
// started; it is closed again as soon as the station connects.
class Network
{
public:
    static constexpr const char *HOSTNAME = "lynx";
    static constexpr const char *AP_NAME = "Lynx-Setup";

    void begin();
    void loop();

    bool isConnected() const;
    bool isPortalActive() const { return portalActive; }

    // Called when a firmware update starts, from ArduinoOTA or the web page
    void onUpdateStart(std::function<void()> callback) { updateStartCallback = callback; }
    // Called for each message posted to /message
    void onMessage(std::function<void(const Message &)> callback) { messageCallback = callback; }
    // Called for each value posted to /brightness (0.05 to 1.0); returns the level applied
    void onBrightness(std::function<float(float)> callback) { brightnessCallback = callback; }
    // Lets the app add its own fields to /status.json
    void onStatus(std::function<void(JsonObject)> callback) { statusCallback = callback; }
    // RGB565 frame served by /screen for the live preview
    void setFrameSource(const uint16_t *buffer, uint16_t width, uint16_t height);

private:
    WebServer server{80};
    DNSServer dns;
    bool servicesStarted = false;
    bool wasConnected = false;
    bool portalActive = false;
    uint32_t connectStartedAt = 0;
    uint32_t restartAt = 0;
    std::function<void()> updateStartCallback;
    std::function<void(const Message &)> messageCallback;
    std::function<float(float)> brightnessCallback;
    std::function<void(JsonObject)> statusCallback;
    bool updateStarted = false;
    String updateError;
    const uint16_t *frame = nullptr;
    uint16_t frameWidth = 0;
    uint16_t frameHeight = 0;

    void startServices();
    void startPortal();
    void stopPortal();
    void scheduleRestart();

    bool isAuthorized();
    bool authorize();
    void sendJson(int code, const JsonDocument &doc);
    void handlePage();
    void handleGetConfig();
    void handlePostConfig();
    void handleStatus();
    void handleScreen();
    void handleMessage();
    void handleBrightness();
    void handleFirmwareUpload();
    void handleFirmwareDone();
    void handleNotFound();
};
