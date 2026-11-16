#include "TimeManager.h"

namespace
{
    constexpr time_t VALID_EPOCH = 1700000000; // Anything earlier means "not synced yet"
    const char *const WEEKDAY_NAMES[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char *const MONTH_NAMES[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
}

void TimeManager::begin(const String &tz)
{
    configTzTime(tz.c_str(), "pool.ntp.org", "time.nist.gov");
}

time_t TimeManager::epoch() const
{
    time_t t = time(nullptr);
    return t >= VALID_EPOCH ? t : 0;
}

bool TimeManager::now(struct tm &out) const
{
    time_t t = epoch();
    if (!t)
    {
        return false;
    }
    localtime_r(&t, &out);
    return true;
}

const char *TimeManager::weekdayName(int wday)
{
    return WEEKDAY_NAMES[wday % 7];
}

const char *TimeManager::monthName(int month)
{
    return MONTH_NAMES[month % 12];
}
