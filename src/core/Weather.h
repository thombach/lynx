#pragma once
// Pure weather helpers, without Arduino dependencies so they can be unit tested natively

namespace core
{
    // Bar height in pixels for an hourly precipitation amount (mm/h), on a fixed scale so a
    // drizzle never looks like a downpour: light, moderate and heavy rain map to 3 steps
    inline int precipitationBarHeight(float millimeters)
    {
        if (millimeters < 0.1f)
            return 0;
        if (millimeters < 1)
            return 2;
        if (millimeters < 4)
            return 4;
        return 6;
    }
}
