#pragma once
#include <Arduino.h>

// Debounced active-low push button
class Button
{
public:
    explicit Button(uint8_t pin) : pin(pin) {}

    void begin() { pinMode(pin, INPUT_PULLUP); }

    // True once per press, on the debounced falling edge
    bool pressed()
    {
        bool reading = digitalRead(pin) == LOW;
        uint32_t now = millis();
        if (reading != lastReading)
        {
            lastReading = reading;
            changedAt = now;
        }
        if (reading != stableState && now - changedAt >= DEBOUNCE_MS)
        {
            stableState = reading;
            return stableState;
        }
        return false;
    }

private:
    static constexpr uint32_t DEBOUNCE_MS = 30;

    const uint8_t pin;
    bool lastReading = false;
    bool stableState = false;
    uint32_t changedAt = 0;
};
