#include "ForecastWeatherScreen.h"
#include <math.h>
#include <Fonts/Picopixel.h>
#include <Fonts/FreeSans9pt7b.h>
#include "managers/DisplayManager.h"

ForecastWeatherScreen::ForecastWeatherScreen(Adafruit_Protomatter *m, WeatherManager *w, TimeManager *t)
{
    matrix = m;
    weatherManager = w;
    timeManager = t;
}

void ForecastWeatherScreen::draw()
{
    matrix->fillScreen(0);
    // Weather images
    const uint16_t *dailyWeatherImage = weatherManager->getBitmap(weatherManager->daysWeatherCode[0], false);
    const uint16_t *nextWeatherImage = weatherManager->getBitmap(weatherManager->daysWeatherCode[1], false);
    displayManager.drawBitmap(dailyWeatherImage, 16, 16, 2, 0);
    displayManager.drawBitmap(nextWeatherImage, 16, 16, 2, 16);

    // Day
    struct tm now;
    if (!timeManager->getLocalTime(&now))
    {
        // couldn’t get time → skip draw
        return;
    }
    matrix->setTextColor(displayManager.color565(255, 255, 255));
    matrix->setFont(&Picopixel);
    matrix->setTextSize(1);
    matrix->setCursor(20, 10);
    matrix->print(timeManager->getWeekdayName(now.tm_wday));
    matrix->setCursor(20, 26);
    matrix->print(timeManager->getWeekdayName(now.tm_wday + 1));

    // Temperature
    int x, y;
    String dailyMinTempStr = String(String((int)round(weatherManager->daysMinTemperature[0])));
    String dailyMaxTempStr = String(String((int)round(weatherManager->daysMaxTemperature[0])));
    String nextMinTempStr = String(String((int)round(weatherManager->daysMinTemperature[1])));
    String nextMaxTempStr = String(String((int)round(weatherManager->daysMaxTemperature[1])));
    u_int16_t minColor = displayManager.color565(0, 207, 255);
    u_int16_t maxColor = displayManager.color565(255, 64, 0);

    matrix->setFont(&Picopixel);
    matrix->setCursor(36, 6);
    matrix->setTextColor(minColor);
    matrix->print(dailyMinTempStr);
    x = matrix->getCursorX();
    y = matrix->getCursorY();
    matrix->drawPixel(x, y - 4, minColor);

    matrix->setCursor(36, 13);
    matrix->setTextColor(maxColor);
    matrix->print(dailyMaxTempStr);
    x = matrix->getCursorX();
    y = matrix->getCursorY();
    matrix->drawPixel(x, y - 4, maxColor);

    matrix->setCursor(36, 22);
    matrix->setTextColor(minColor);
    matrix->print(nextMinTempStr);
    x = matrix->getCursorX();
    y = matrix->getCursorY();
    matrix->drawPixel(x, y - 4, minColor);

    matrix->setCursor(36, 29);
    matrix->setTextColor(maxColor);
    matrix->print(nextMaxTempStr);
    x = matrix->getCursorX();
    y = matrix->getCursorY();
    matrix->drawPixel(x, y - 4, maxColor);

    // Precipitations
    String dailyPrecipitationSumStr = String((int)round(weatherManager->daysPrecipitationSum[0])) + String(" m");
    String nextPrecipitationSumStr = String((int)round(weatherManager->daysPrecipitationSum[1])) + String(" m");
    matrix->setTextColor(displayManager.color565(255, 255, 255));
    matrix->setCursor(47, 10);
    matrix->print(dailyPrecipitationSumStr);

    matrix->setCursor(47, 26);
    matrix->print(nextPrecipitationSumStr);

    matrix->show();
    weatherManager->setDirty(false);
}

void ForecastWeatherScreen::update()
{
    if (weatherManager->isDirty())
    {
        draw();
    }
    weatherManager->update();
    timeManager->update();
}
