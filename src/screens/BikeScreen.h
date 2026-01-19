#pragma once

#include <Adafruit_Protomatter.h>
#include "../managers/BikeManager.h"
#include "Screen.h"

class BikeScreen : public Screen
{
public:
    BikeScreen(Adafruit_Protomatter *matrix, BikeManager *bikeManager);
    void draw() override;
    void update() override;

private:
    Adafruit_Protomatter *matrix;
    BikeManager *bikeManager;
};
