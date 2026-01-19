#include "TimeManager.h"
#include <WiFi.h>

static const char *const WEEKDAY_NAMES[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char *const MONTH_NAMES[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

TimeManager::TimeManager() : config(ConfigManager::getInstance())
{
}

void TimeManager::begin()
{
  syncNTP();
  lastSync = millis();
}

void TimeManager::update()
{
  if (millis() - lastSync > syncInterval)
  {
    syncNTP();
    lastSync = millis();
  }
}

// Sync time with NTP servers
void TimeManager::syncNTP()
{
  Serial.println("Syncing time with NTP...");
  configTime(config.gmtOffset, 0, "pool.ntp.org", "time.nist.gov");
  delay(2000);
  struct tm timeinfo;
  if (::getLocalTime(&timeinfo))
  {
    Serial.println("Time sync successful!");
  }
  else
  {
    Serial.println("Time sync failed!");
  }
}

// Compute if DST is active (for Europe)
bool TimeManager::isDST(struct tm *timeinfo)
{
  int month = timeinfo->tm_mon + 1; // tm_mon = 0–11
  int day = timeinfo->tm_mday;
  int wday = timeinfo->tm_wday; // Sunday = 0
  if (month < 3 || month > 10)
    return false; // Before March or after October → no DST
  if (month > 3 && month < 10)
    return true;               // Between April and September → DST
  int lastSunday = day - wday; // For March or October → check the last Sunday
  if (month == 3)
    return (lastSunday >= 25); // DST starts last Sunday of March
  if (month == 10)
    return (lastSunday < 25); // DST ends last Sunday of October
  return false;
}

bool TimeManager::getLocalTime(struct tm *outTime)
{
  // Get time
  struct tm timeinfo;
  if (!::getLocalTime(&timeinfo))
  {
    Serial.println("Failed to get time");
    return false;
  }

  // Apply DST logic
  bool dstActive = isDST(&timeinfo);
  time_t rawtime;
  time(&rawtime);
  if (dstActive)
    rawtime += config.dstOffset;
  *outTime = *localtime(&rawtime);

  return true;
}

const char *TimeManager::getWeekdayName(const int wday)
{
  return WEEKDAY_NAMES[wday % 7];
}

const char *TimeManager::getMonthName(const struct tm *timeinfo)
{
  return MONTH_NAMES[timeinfo->tm_mon % 12];
}