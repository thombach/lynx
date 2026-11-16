#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include <Fonts/Picopixel.h>
#include <esp_timer.h>
#include <algorithm>
#include <atomic>
#include <vector>
#include "managers/AirQualityManager.h"
#include "managers/BikeManager.h"
#include "managers/BrightnessManager.h"
#include "managers/ConfigManager.h"
#include "managers/DisplayManager.h"
#include "managers/TimeManager.h"
#include "managers/TransportManager.h"
#include "managers/WeatherManager.h"
#include "net/Network.h"
#include "screens/BikeScreen.h"
#include "screens/ClockScreen.h"
#include "screens/CurrentWeatherScreen.h"
#include "screens/ForecastWeatherScreen.h"
#include "screens/HourlyScreen.h"
#include "screens/SunScreen.h"
#include "screens/TransportScreen.h"
#include "ui/BootAnimation.h"
#include "ui/Button.h"
#include "ui/ScrollText.h"
#include "ui/TapSensor.h"
#include "ui/Text.h"
#include "ui/Transition.h"
#include "util/Timer.h"

#ifndef LYNX_VERSION
#define LYNX_VERSION "dev"
#endif

namespace
{
    constexpr uint32_t FRAME_MS = 40;
    constexpr uint32_t OVERLAY_MS = 1500;
    // Pressing or releasing a button also shakes the case: a tap gesture is only accepted
    // once it has been confirmed that no button was touched around it
    constexpr uint32_t TAP_CONFIRM_MS = 120;
    constexpr uint32_t TAP_BUTTON_QUIET_MS = 600;
    constexpr uint32_t FETCH_PERIOD_MS = 1000;
    constexpr uint32_t FETCH_STACK_SIZE = 16384;

    // Matrix Portal S3 pin mapping
    uint8_t rgbPins[] = {42, 40, 41, 38, 37, 39};
    uint8_t addrPins[] = {45, 36, 48, 35};
    constexpr uint8_t CLOCK_PIN = 2, LATCH_PIN = 47, OE_PIN = 14;
    constexpr uint8_t BIT_DEPTH = 5; // Each extra bit halves the refresh rate: 6 visibly flickers

    Adafruit_Protomatter matrix(64, BIT_DEPTH, 1, rgbPins, 4, addrPins, CLOCK_PIN, LATCH_PIN, OE_PIN, true);

    ConfigManager &config = ConfigManager::getInstance();
    DisplayManager display;
    TimeManager timeManager;
    BrightnessManager brightness(timeManager);
    Network network;
    Transition transition;
    TapSensor tapSensor;

    WeatherManager weather;
    AirQualityManager airQuality;
    BikeManager bikes;
    TransportManager transport;

    ClockScreen clockScreen(display, timeManager);
    CurrentWeatherScreen currentWeatherScreen(display, weather, airQuality);
    ForecastWeatherScreen forecastWeatherScreen(display, weather, timeManager);
    HourlyScreen hourlyScreen(display, weather);
    SunScreen sunScreen(display, weather);
    BikeScreen bikeScreen(display, bikes);
    TransportScreen transportScreen(display, transport, timeManager);

    Button upButton(PIN_BUTTON_UP);
    Button downButton(PIN_BUTTON_DOWN);

    struct Slot
    {
        const char *name;
        Screen *screen;
        uint32_t duration;
    };
    std::vector<Slot> rotation;
    std::vector<Manager *> managers;
    size_t currentSlot = 0;
    uint32_t shownAt = 0;
    bool paused = false; // Rotation stopped on the current screen
    core::Gesture pendingGesture = core::Gesture::None;
    Timer brightnessOverlay;
    Timer pauseOverlay;
    std::atomic<bool> updating{false};

    Message message;
    Timer messageTimer;
    ScrollText messageText{64, ScrollText::Font::Large, ScrollText::Mode::Marquee};
    bool wasShowingMessage = false;

    // Builds the screen rotation from the config and enables only the managers it needs
    void setupRotation()
    {
        struct Available
        {
            const char *name;
            Screen *screen;
            bool enabled;
            std::vector<Manager *> needs;
        };
        const Available available[] = {
            {"clock", &clockScreen, true, {}},
            {"weather", &currentWeatherScreen, config.hasWeather(), {&weather, &airQuality}},
            {"forecast", &forecastWeatherScreen, config.hasWeather(), {&weather}},
            {"hourly", &hourlyScreen, config.hasWeather(), {&weather}},
            {"sun", &sunScreen, config.hasWeather(), {&weather}},
            {"bike", &bikeScreen, config.hasBike(), {&bikes}},
            {"transport", &transportScreen, config.hasTransport(), {&transport}},
        };

        for (const ScreenEntry &entry : config.screens)
        {
            const Available *match = nullptr;
            for (const Available &candidate : available)
            {
                if (entry.name == candidate.name)
                {
                    match = &candidate;
                }
            }
            if (!match || !match->enabled)
            {
                Serial.printf("[screens] Skipping \"%s\" (unknown or not configured)\n", entry.name.c_str());
                continue;
            }

            rotation.push_back({match->name, match->screen, entry.duration});
            for (Manager *manager : match->needs)
            {
                if (std::find(managers.begin(), managers.end(), manager) == managers.end())
                {
                    managers.push_back(manager);
                }
            }
        }

        if (rotation.empty())
        {
            rotation.push_back({"clock", &clockScreen, config.screenInterval});
        }
    }

    // Runs on core 0 so slow network calls never freeze the display or the buttons
    void fetchTask(void *)
    {
        for (;;)
        {
            if (!updating && network.isConnected())
            {
                for (Manager *manager : managers)
                {
                    manager->tick();
                }
            }
            vTaskDelay(pdMS_TO_TICKS(FETCH_PERIOD_MS));
        }
    }

    void showSlot(size_t index, const char *reason)
    {
        transition.start();
        currentSlot = index % rotation.size();
        Serial.printf("[screens] %s (%s)\n", rotation[currentSlot].name, reason);
        shownAt = millis();
        rotation[currentSlot].screen->onShow();
    }

    void showBrightness()
    {
        pauseOverlay.stop();
        brightnessOverlay.start(OVERLAY_MS);
    }

    void togglePause()
    {
        paused = !paused;
        Serial.printf("[screens] %s\n", paused ? "Paused" : "Resumed");
        shownAt = millis(); // The current screen gets a full duration on resume
        brightnessOverlay.stop();
        pauseOverlay.start(OVERLAY_MS);
    }

    // Tap gesture, once confirmed that it doesn't come from a button
    core::Gesture readGesture(uint32_t now)
    {
        core::Gesture gesture = tapSensor.poll();
        if (gesture != core::Gesture::None)
        {
            pendingGesture = gesture;
        }
        if (pendingGesture == core::Gesture::None)
        {
            return core::Gesture::None;
        }

        // Any button contact from shortly before the first tap until now
        uint32_t window = now - tapSensor.startedAt() + TAP_BUTTON_QUIET_MS;
        if (upButton.isTouched() || downButton.isTouched() ||
            now - upButton.lastChangeAt() < window || now - downButton.lastChangeAt() < window)
        {
            pendingGesture = core::Gesture::None;
            return core::Gesture::None;
        }
        if (now - tapSensor.endedAt() < TAP_CONFIRM_MS)
        {
            return core::Gesture::None;
        }
        gesture = pendingGesture;
        pendingGesture = core::Gesture::None;
        return gesture;
    }

    void handleInput()
    {
        bool up = upButton.pressed();
        bool down = downButton.pressed();
        core::Gesture gesture = readGesture(millis());

        if (!up && !down && gesture == core::Gesture::None)
        {
            return;
        }

        if (brightness.wake())
        {
            return; // During night mode, the first action only lights the display up
        }
        if (up || down)
        {
            // Adjusting the brightness keeps a message on screen
            brightness.step((up ? 1 : 0) - (down ? 1 : 0));
            showBrightness();
            return;
        }
        if (messageTimer.isRunning())
        {
            messageTimer.stop(); // Any tap dismisses the message
            return;
        }
        if (gesture == core::Gesture::Double)
        {
            togglePause();
        }
        else
        {
            showSlot(currentSlot + 1, "tap");
        }
    }

    // Brightness set from the web page
    float setBrightness(float value)
    {
        brightness.setUserLevel(value);
        brightness.wake(); // Shows the new level during night mode
        showBrightness();
        return brightness.userLevel();
    }

    void showMessage(const Message &received)
    {
        Serial.printf("[message] %s\n", received.text.c_str());
        message = received;
        message.text = toAscii(message.text);
        messageText.reset();
        messageTimer.start(message.duration);
        brightness.wake();
        transition.start();
    }

    void drawPostedMessage()
    {
        Adafruit_Protomatter &gfx = display.gfx();
        uint16_t color = display.color(message.color);

        gfx.setFont(&Picopixel);
        gfx.setTextSize(1);
        gfx.setTextColor(color);
        display.drawCentered("Message", 6);

        messageText.draw(display, message.text, 0, 13, color, true);

        // Remaining display time
        float remaining = 1.0f - min(1.0f, (float)messageTimer.elapsed() / messageTimer.total());
        gfx.drawFastHLine(0, gfx.height() - 1, lroundf(gfx.width() * remaining), color);
    }

    void fillStatus(JsonObject status)
    {
        status["version"] = LYNX_VERSION;
        status["uptime"] = (uint32_t)(esp_timer_get_time() / 1000000);
        status["heapSize"] = ESP.getHeapSize();
        status["freeHeap"] = ESP.getFreeHeap();
        status["minFreeHeap"] = ESP.getMinFreeHeap();
        status["screen"] = rotation[currentSlot].name;
        status["paused"] = paused;
        status["brightness"] = brightness.userLevel();
        status["night"] = brightness.isNight();
        status["tapSensor"] = tapSensor.isAvailable();

        struct tm now;
        char time[20];
        if (timeManager.now(now))
        {
            strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", &now);
            status["time"] = time;
        }

        JsonArray sources = status["sources"].to<JsonArray>();
        for (const Manager *manager : managers)
        {
            ManagerStatus s = manager->status();
            JsonObject source = sources.add<JsonObject>();
            source["name"] = s.name;
            source["ok"] = s.hasData;
            source["stale"] = s.stale;
            if (s.hasData)
            {
                source["lastSync"] = s.lastSuccessAgo / 1000;
            }
            source["failures"] = s.failures;
            if (s.failures > 0)
            {
                source["error"] = s.lastError;
            }
        }
    }

    void onUpdateStart()
    {
        updating = true; // Pauses fetching and screen rendering until the restart
        display.setBrightness(brightness.current());
        display.clear();
        display.drawMessage("Updating...");
        display.show();
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.printf("Lynx %s is starting...\n", LYNX_VERSION);

    ProtomatterStatus status = matrix.begin();
    if (status != PROTOMATTER_OK)
    {
        // Nothing can be displayed without the matrix
        Serial.printf("Matrix init failed: %d\n", status);
        for (;;)
        {
            delay(1000);
        }
    }
    display.begin(matrix, BIT_DEPTH);
    transition.begin(matrix);

    bool configLoaded = config.load();
    brightness.begin();
    display.setBrightness(brightness.current());
    if (!configLoaded)
    {
        Serial.println("No valid config, the setup portal will open");
    }

    network.onUpdateStart(onUpdateStart);
    network.onMessage(showMessage);
    network.onBrightness(setBrightness);
    network.onStatus(fillStatus);
    network.setFrameSource(matrix.getBuffer(), matrix.width(), matrix.height());
    network.begin();
    timeManager.begin(config.timezone);

    upButton.begin();
    downButton.begin();
    tapSensor.begin(config.tapThreshold);

    setupRotation();
    xTaskCreatePinnedToCore(fetchTask, "fetch", FETCH_STACK_SIZE, nullptr, 1, nullptr, 0);

    // Wi-Fi connects in the background while the logo plays
    BootAnimation::play(display);
    showSlot(0, "boot");
}

void loop()
{
    uint32_t frameStart = millis();

    network.loop(); // Web requests see the last complete frame
    if (updating)
    {
        return;
    }
    handleInput();

    bool showingMessage = messageTimer.isRunning();
    if (wasShowingMessage && !showingMessage)
    {
        transition.start(); // Slide back to the current screen, with a fresh duration
        shownAt = millis();
    }
    wasShowingMessage = showingMessage;

    if (!showingMessage && !paused && millis() - shownAt >= rotation[currentSlot].duration)
    {
        showSlot(currentSlot + 1, "timer");
    }

    display.setBrightness(brightness.current());
    display.clear();
    if (network.isPortalActive())
    {
        display.drawMessage("Wi-Fi setup", Network::AP_NAME);
    }
    else if (showingMessage)
    {
        drawPostedMessage();
    }
    else
    {
        rotation[currentSlot].screen->draw();
        if (paused)
        {
            display.drawPauseMarker();
        }
    }
    transition.apply();
    if (brightnessOverlay.isRunning())
    {
        display.drawBrightnessOverlay(brightness.userLevel());
    }
    else if (pauseOverlay.isRunning())
    {
        display.drawBanner(paused ? "Paused" : "Resumed");
    }
    display.show();

    uint32_t elapsed = millis() - frameStart;
    if (elapsed < FRAME_MS)
    {
        delay(FRAME_MS - elapsed);
    }
}
