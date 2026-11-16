#pragma once
#include <Arduino.h>
#include "TimeManager.h"
#include "util/Timer.h"

// User brightness (persisted in NVS) combined with the scheduled night mode
class BrightnessManager
{
public:
    explicit BrightnessManager(TimeManager &time) : time(time) {}

    void begin();
    void step(int delta);            // delta * 5%, between 5% and 100%
    void setUserLevel(float value); // 0.05 to 1.0, rounded to 5% steps
    // During night mode, wakes the display for a while; returns true if it was asleep
    bool wake();
    float current();
    float userLevel() const { return (float)level / LEVELS; } // Ignores night mode
    bool isNight() const;

private:
    static constexpr int LEVELS = 20;
    static constexpr uint32_t WAKE_MS = 60000;

    void setLevel(int value);

    TimeManager &time;
    int level = LEVELS;
    Timer wakeTimer;
};
