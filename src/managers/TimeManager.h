#pragma once
#include <time.h>
#include "ConfigManager.h"
#include "Manager.cpp"

class TimeManager : public Manager
{
public:
  TimeManager();
  void begin();
  void update();
  bool getLocalTime(struct tm *outTime);
  static const char *getWeekdayName(const int wday);
  static const char *getMonthName(const struct tm *timeinfo);

private:
  ConfigManager &config;
  unsigned long lastSync = 0;
  const unsigned long syncInterval = 6UL * 60UL * 60UL * 1000UL; // every 6h
  bool isDST(struct tm *timeinfo);
  void syncNTP();
};