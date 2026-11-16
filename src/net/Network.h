#pragma once
#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <functional>

// Wi-Fi connection, setup portal, web config page and OTA updates.
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

private:
    WebServer server{80};
    DNSServer dns;
    bool servicesStarted = false;
    bool wasConnected = false;
    bool portalActive = false;
    uint32_t connectStartedAt = 0;
    uint32_t restartAt = 0;
    std::function<void()> updateStartCallback;
    bool updateStarted = false;
    String updateError;

    void startServices();
    void startPortal();
    void stopPortal();
    void scheduleRestart();

    bool isAuthorized();
    bool authorize();
    void handlePage();
    void handleGetConfig();
    void handlePostConfig();
    void handleFirmwareUpload();
    void handleFirmwareDone();
    void handleNotFound();
};
