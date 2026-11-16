#pragma once

// Screens redraw a full frame every tick from the latest published data.
// The caller clears the display before draw() and shows it afterwards.
class Screen
{
public:
    virtual ~Screen() = default;
    virtual void onShow() {}
    virtual void draw() = 0;
};
