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
    void cycle(); // +5%, wraps around to the minimum
    // During night mode, wakes the display for a while; returns true if it was asleep
    bool wake();
    float current();
    float userLevel() const { return (float)level / LEVELS; } // Ignores night mode

private:
    static constexpr int LEVELS = 20;
    static constexpr uint32_t WAKE_MS = 60000;

    TimeManager &time;
    int level = LEVELS;
    Timer wakeTimer;

    bool isNight() const;
};
