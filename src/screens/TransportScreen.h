#pragma once

#include <Adafruit_Protomatter.h>
#include "../managers/TransportManager.h"
#include "Screen.h"

class TransportScreen : public Screen
{
public:
    TransportScreen(Adafruit_Protomatter *matrix, TransportManager *transportManager);
    void draw() override;
    void update() override;

private:
    Adafruit_Protomatter *matrix;
    TransportManager *transportManager;
};
