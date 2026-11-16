#pragma once
#include <Arduino.h>

// Debounced active-low push button, with auto-repeat while held
class Button
{
public:
    explicit Button(uint8_t pin) : pin(pin) {}

    void begin() { pinMode(pin, INPUT_PULLUP); }

    // True once per press, on the debounced falling edge, then repeatedly while held
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
            repeatAt = now + REPEAT_DELAY_MS;
            return stableState;
        }
        if (stableState && (int32_t)(now - repeatAt) >= 0)
        {
            repeatAt = now + REPEAT_MS;
            return true;
        }
        return false;
    }

    // Raw state, without debouncing: true from the very first contact
    bool isTouched() const { return lastReading; }
    // Time of the last raw press or release
    uint32_t lastChangeAt() const { return changedAt; }

private:
    static constexpr uint32_t DEBOUNCE_MS = 30;
    static constexpr uint32_t REPEAT_DELAY_MS = 400;
    static constexpr uint32_t REPEAT_MS = 120;

    const uint8_t pin;
    bool lastReading = false;
    bool stableState = false;
    uint32_t changedAt = 0;
    uint32_t repeatAt = 0;
};
