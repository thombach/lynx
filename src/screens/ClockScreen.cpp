#include "ClockScreen.h"
#include <time.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/Picopixel.h>
#include "managers/DisplayManager.h"

const char *const WEEKDAY_NAMES[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char *const MONTH_NAMES[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

ClockScreen::ClockScreen(Adafruit_Protomatter *m, TimeManager *t)
{
  matrix = m;
  timeManager = t;
}

void ClockScreen::draw()
{
  struct tm now;
  if (!timeManager->getLocalTime(&now))
  {
    // couldn’t get time → skip draw
    return;
  }
  // Blink colon every second
  if (millis() - lastBlink >= 1000)
  {
    colonVisible = !colonVisible;
    lastBlink = millis();
  }

  // Time coordinates
  char timeStr[6];
  snprintf(timeStr, sizeof(timeStr), colonVisible ? "%02d:%02d" : "%02d %02d", now.tm_hour, now.tm_min);
  int16_t x1, y1;
  uint16_t w1, h1;
  matrix->setFont(&FreeSans12pt7b);
  matrix->getTextBounds(timeStr, 0, 0, &x1, &y1, &w1, &h1);
  int16_t xTime = ((matrix->width() - w1) / 2) - x1;
  int16_t yTime = h1 + 2;

  matrix->fillScreen(0);
  matrix->setCursor(xTime, yTime);
  matrix->setTextColor(displayManager.color565(255, 255, 255));
  matrix->print(timeStr);

  // Date coordinates
  char dateStr[16];
  snprintf(dateStr, sizeof(dateStr), "%s %d %s %d", TimeManager::getWeekdayName(now.tm_wday), now.tm_mday, TimeManager::getMonthName(&now), now.tm_year + 1900);
  int16_t x2, y2;
  uint16_t w2, h2;
  matrix->setFont(&Picopixel);
  matrix->getTextBounds(dateStr, 0, 0, &x2, &y2, &w2, &h2);
  int16_t xDate = ((matrix->width() - w2) / 2) - x2;
  int16_t yDate = matrix->height() - 4;
  matrix->setCursor(xDate, yDate);
  matrix->setTextColor(displayManager.color565(255, 128, 0));
  matrix->print(dateStr);

  matrix->show();
}

void ClockScreen::update()
{
  draw();
  timeManager->update();
}
