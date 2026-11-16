#pragma once
#include <Arduino.h>
#include "managers/DisplayManager.h"

// Logo shown at startup: the letters drop in one by one and bounce, an underline grows
// from the center, then a shine sweeps across. The last frame stays in the buffer so the
// first screen slides in over it.
class BootAnimation
{
public:
    // Blocks for DURATION_MS
    static void play(DisplayManager &display);

    // The animation itself ends at 1400 ms, then the logo stays still for HOLD_MS so the
    // first data has time to load before the screens start
    static constexpr uint32_t HOLD_MS = 1600;
    static constexpr uint32_t DURATION_MS = 1400 + HOLD_MS;

private:
    static void drawFrame(DisplayManager &display, uint32_t t);
};
