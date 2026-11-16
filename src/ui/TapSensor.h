#pragma once
#include <Arduino.h>
#include <Adafruit_LIS3DH.h>
#include "core/TapGesture.h"

// Detects single and double taps on the case with the Matrix Portal's onboard LIS3DH accelerometer
class TapSensor
{
public:
    // threshold: 1-127, lower is more sensitive; 0 disables detection
    bool begin(uint8_t threshold);
    // Call every frame. A single tap is reported once the double tap delay has passed
    core::Gesture poll();
    // Times of the first and last tap of the gesture poll() just returned
    uint32_t startedAt() const { return gesture.startedAt(); }
    uint32_t endedAt() const { return gesture.endedAt(); }
    bool isAvailable() const { return available; }

private:
    Adafruit_LIS3DH accelerometer;
    core::TapGesture gesture;
    bool available = false;
};
