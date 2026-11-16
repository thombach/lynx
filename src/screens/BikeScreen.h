#pragma once
#include "Screen.h"
#include "managers/BikeManager.h"
#include "managers/DisplayManager.h"
#include "ui/ScrollText.h"

class BikeScreen : public Screen
{
public:
    BikeScreen(DisplayManager &display, BikeManager &bikes) : display(display), bikes(bikes) {}
    void onShow() override { stationName.reset(); }
    void draw() override;

private:
    DisplayManager &display;
    BikeManager &bikes;
    ScrollText stationName{60};

    void drawCount(const Bitmap &icon, int value, int16_t y);
};
