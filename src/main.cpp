#include <Arduino.h>
#include <Adafruit_Protomatter.h>
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
#include "screens/TransportScreen.h"
#include "ui/Button.h"
#include "util/Timer.h"

namespace
{
    constexpr uint32_t FRAME_MS = 40;
    constexpr uint32_t SPLASH_MS = 1500;
    constexpr uint32_t BRIGHTNESS_OVERLAY_MS = 1500;
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

    WeatherManager weather;
    AirQualityManager airQuality;
    BikeManager bikes;
    TransportManager transport;

    ClockScreen clockScreen(display, timeManager);
    CurrentWeatherScreen currentWeatherScreen(display, weather, airQuality);
    ForecastWeatherScreen forecastWeatherScreen(display, weather, timeManager);
    BikeScreen bikeScreen(display, bikes);
    TransportScreen transportScreen(display, transport, timeManager);

    Button upButton(PIN_BUTTON_UP);
    Button downButton(PIN_BUTTON_DOWN);

    struct Slot
    {
        Screen *screen;
        uint32_t duration;
    };
    std::vector<Slot> rotation;
    std::vector<Manager *> managers;
    size_t currentSlot = 0;
    uint32_t shownAt = 0;
    Timer brightnessOverlay;
    std::atomic<bool> updating{false};

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

            rotation.push_back({match->screen, entry.duration});
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
            rotation.push_back({&clockScreen, config.screenInterval});
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

    void showSlot(size_t index)
    {
        currentSlot = index % rotation.size();
        shownAt = millis();
        rotation[currentSlot].screen->onShow();
    }

    void handleButtons()
    {
        bool up = upButton.pressed();
        bool down = downButton.pressed();
        if (!up && !down)
        {
            return;
        }
        if (brightness.wake())
        {
            return; // During night mode, the first press only lights the display up
        }
        if (up)
        {
            brightness.cycle();
            brightnessOverlay.start(BRIGHTNESS_OVERLAY_MS);
        }
        if (down)
        {
            showSlot(currentSlot + 1);
        }
    }

    void drawSplash()
    {
        Adafruit_Protomatter &gfx = display.gfx();
        const char *text = "Lynx";
        const uint16_t colors[] = {display.color(0, 255, 64), display.color(255, 128, 0),
                                   display.color(0, 64, 255), display.color(255, 0, 255)};
        int16_t x1, y1;
        uint16_t w, h;

        display.clear();
        gfx.setFont(nullptr);
        gfx.setTextSize(1);
        gfx.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
        gfx.setCursor((gfx.width() - w) / 2, (gfx.height() - h) / 2);
        for (int i = 0; i < 4; i++)
        {
            gfx.setTextColor(colors[i]);
            gfx.print(text[i]);
        }
        display.show();
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
    Serial.println("Lynx is starting...");

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

    bool configLoaded = config.load();
    brightness.begin();
    display.setBrightness(brightness.current());
    drawSplash();
    if (!configLoaded)
    {
        Serial.println("No valid config, the setup portal will open");
    }

    network.onUpdateStart(onUpdateStart);
    network.begin();
    timeManager.begin(config.timezone);

    upButton.begin();
    downButton.begin();

    setupRotation();
    xTaskCreatePinnedToCore(fetchTask, "fetch", FETCH_STACK_SIZE, nullptr, 1, nullptr, 0);

    delay(SPLASH_MS);
    showSlot(0);
}

void loop()
{
    uint32_t frameStart = millis();

    network.loop();
    if (updating)
    {
        return;
    }
    handleButtons();

    if (millis() - shownAt >= rotation[currentSlot].duration)
    {
        showSlot(currentSlot + 1);
    }

    display.setBrightness(brightness.current());
    display.clear();
    if (network.isPortalActive())
    {
        display.drawMessage("Wi-Fi setup", Network::AP_NAME);
    }
    else
    {
        rotation[currentSlot].screen->draw();
    }
    if (brightnessOverlay.isRunning())
    {
        display.drawBrightnessOverlay(brightness.userLevel());
    }
    display.show();

    uint32_t elapsed = millis() - frameStart;
    if (elapsed < FRAME_MS)
    {
        delay(FRAME_MS - elapsed);
    }
}
