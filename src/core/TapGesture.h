#pragma once
// Single / double tap recognition from raw tap events, without Arduino dependencies so it can
// be unit tested natively
#include <cstdint>

namespace core
{
    enum class Gesture
    {
        None,
        Single,
        Double,
    };

    class TapGesture
    {
    public:
        // A knock makes the case vibrate: taps this soon after the previous one are ignored
        static constexpr uint32_t RING_MS = 150;
        // A second tap within this delay makes a double tap. A single tap is only reported
        // once it has passed
        static constexpr uint32_t DOUBLE_TAP_MS = 400;

        // Called every frame with whether the sensor saw a tap since the previous call
        Gesture update(uint32_t now, bool tapped)
        {
            if (tapped && hasTapped && now - lastAt < RING_MS)
            {
                tapped = false;
            }
            if (tapped)
            {
                hasTapped = true;
                lastAt = now;
                if (pending)
                {
                    pending = false;
                    return Gesture::Double;
                }
                pending = true;
                firstAt = now;
                return Gesture::None;
            }
            if (pending && now - firstAt >= DOUBLE_TAP_MS)
            {
                pending = false;
                return Gesture::Single;
            }
            return Gesture::None;
        }

        // Times of the first and last tap of the gesture update() just returned
        uint32_t startedAt() const { return firstAt; }
        uint32_t endedAt() const { return lastAt; }

    private:
        bool pending = false;
        bool hasTapped = false;
        uint32_t firstAt = 0;
        uint32_t lastAt = 0;
    };
}
