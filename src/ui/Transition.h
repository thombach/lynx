#pragma once
#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include <memory>
#include "util/Timer.h"

// Slides the newly drawn frame in from the right, over the previous one
class Transition
{
public:
    void begin(Adafruit_Protomatter &matrix);
    // Captures the frame currently displayed; call before drawing the new screen
    void start();
    // Composites the new frame with the captured one; call after drawing, before show()
    void apply();

private:
    static constexpr uint32_t DURATION_MS = 350;

    Adafruit_Protomatter *matrix = nullptr;
    std::unique_ptr<uint16_t[]> previous;
    std::unique_ptr<uint16_t[]> next;
    Timer timer;
};
