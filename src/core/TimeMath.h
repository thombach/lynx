#pragma once
// Pure time helpers, without Arduino dependencies so they can be unit tested natively
#include <cstdio>
#include <ctime>

namespace core
{
    // "HH:MM" -> minutes since midnight, -1 if missing or invalid
    inline int parseClock(const char *text)
    {
        int hours, minutes;
        char extra;
        if (!text || sscanf(text, "%d:%d%c", &hours, &minutes, &extra) != 2 ||
            hours < 0 || hours > 23 || minutes < 0 || minutes > 59)
        {
            return -1;
        }
        return hours * 60 + minutes;
    }

    // True if minute is within [start, end); the window may span midnight
    inline bool inWindow(int minute, int start, int end)
    {
        if (start < 0 || end < 0 || start == end)
        {
            return false;
        }
        if (start < end)
        {
            return minute >= start && minute < end;
        }
        return minute >= start || minute < end;
    }

    // "YYYY-MM-DD HH:MM[:SS]" or "YYYY-MM-DDTHH:MM", in local time -> epoch, 0 if invalid
    inline time_t parseLocalDateTime(const char *text)
    {
        struct tm t = {};
        if (!text || sscanf(text, "%d-%d-%d%*c%d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday,
                            &t.tm_hour, &t.tm_min, &t.tm_sec) < 5)
        {
            return 0;
        }
        t.tm_year -= 1900;
        t.tm_mon -= 1;
        t.tm_isdst = -1;
        return mktime(&t);
    }

    // Whole minutes until `at`: 0 when imminent, -1 once gone for more than `graceSeconds`
    inline long minutesUntil(time_t at, time_t now, long graceSeconds = 30)
    {
        long seconds = (long)(at - now);
        if (seconds < -graceSeconds)
        {
            return -1;
        }
        return seconds < 60 ? 0 : seconds / 60;
    }
}
