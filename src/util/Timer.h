#pragma once
#include <Arduino.h>

// One-shot timer, safe across millis() overflow (expiry is latched when checked)
class Timer
{
public:
    void start(uint32_t duration)
    {
        startedAt = millis();
        this->duration = duration;
        running = true;
    }

    bool isRunning()
    {
        if (running && millis() - startedAt >= duration)
        {
            running = false;
        }
        return running;
    }

private:
    uint32_t startedAt = 0;
    uint32_t duration = 0;
    bool running = false;
};
