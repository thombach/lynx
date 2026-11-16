#include "TapSensor.h"

namespace
{
    constexpr uint8_t I2C_ADDRESS = 0x19;
    constexpr uint8_t SINGLE_TAP = 1; // Double taps are recognized in software, see core::TapGesture
    constexpr uint8_t LATCH = 0x80;   // CLICK_THS bit 7: keeps the event until read, so polling can't miss it
    constexpr uint8_t SOURCE_ACTIVE = 0x40;
    constexpr uint8_t SOURCE_SINGLE = 0x10;
}

bool TapSensor::begin(uint8_t threshold)
{
    if (threshold == 0)
    {
        return false;
    }
    if (!accelerometer.begin(I2C_ADDRESS))
    {
        Serial.println("[tap] Accelerometer not found");
        return false;
    }
    accelerometer.setRange(LIS3DH_RANGE_2_G);
    accelerometer.setClick(SINGLE_TAP, min<uint8_t>(threshold, 127) | LATCH);
    available = true;
    return true;
}

core::Gesture TapSensor::poll()
{
    if (!available)
    {
        return core::Gesture::None;
    }
    uint8_t source = accelerometer.getClick(); // Always read, to clear the latched event
    bool tapped = (source & SOURCE_ACTIVE) && (source & SOURCE_SINGLE);
    return gesture.update(millis(), tapped);
}
