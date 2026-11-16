#include "Transition.h"

void Transition::begin(Adafruit_Protomatter &m)
{
    matrix = &m;
    size_t pixels = matrix->width() * matrix->height();
    previous.reset(new uint16_t[pixels]);
    next.reset(new uint16_t[pixels]);
}

void Transition::start()
{
    memcpy(previous.get(), matrix->getBuffer(), matrix->width() * matrix->height() * sizeof(uint16_t));
    timer.start(DURATION_MS);
}

void Transition::apply()
{
    if (!timer.isRunning())
    {
        return;
    }

    int16_t width = matrix->width();
    int16_t height = matrix->height();
    uint16_t *frame = matrix->getBuffer();
    memcpy(next.get(), frame, width * height * sizeof(uint16_t));

    float t = min(1.0f, (float)timer.elapsed() / timer.total());
    float eased = 1 - powf(1 - t, 3); // Ease-out: fast start, gentle stop
    int16_t shift = lroundf(eased * width);

    for (int16_t row = 0; row < height; row++)
    {
        uint16_t *dest = frame + row * width;
        memcpy(dest, previous.get() + row * width + shift, (width - shift) * sizeof(uint16_t));
        memcpy(dest + width - shift, next.get() + row * width, shift * sizeof(uint16_t));
    }
}
