#pragma once
#include <Arduino.h>
#include <time.h>

class TimeManager
{
public:
    // tz is a POSIX TZ string; SNTP then resyncs on its own and handles DST
    void begin(const String &tz);
    // Non-blocking: false until the clock has been synced at least once
    bool now(struct tm &out) const;
    time_t epoch() const;

    static const char *weekdayName(int wday);
    static const char *monthName(int month);
};
