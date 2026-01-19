#pragma once
#include <Arduino.h>
#include <Adafruit_Protomatter.h>
#include "ConfigManager.h"

class DisplayManager
{
public:
    DisplayManager();
    void begin(Adafruit_Protomatter *matrix);

    void drawBitmap(const uint16_t *bitmap, int w, int h, int x, int y);

    uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

private:
    Adafruit_Protomatter *matrix;
    float brightness;

    // LUT tables (8-bit -> gamma corrected -> scaled -> back to 8-bit)
    uint8_t gammaLUT[256];
    uint8_t brightnessLUT[256];

    void buildLUTs();
    uint16_t rgbTo565(uint8_t r, uint8_t g, uint8_t b);
};
extern DisplayManager displayManager;