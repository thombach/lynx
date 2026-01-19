#include "CurrentWeatherScreen.h"
#include <math.h>
#include <Fonts/Picopixel.h>
#include "managers/DisplayManager.h"

CurrentWeatherScreen::CurrentWeatherScreen(Adafruit_Protomatter *m, WeatherManager *w)
{
    matrix = m;
    weatherManager = w;
}

void CurrentWeatherScreen::draw()
{
    matrix->fillScreen(0);
    // Weather image
    const uint16_t *weatherImage = weatherManager->getBitmap(weatherManager->currentWeatherCode, true);
    displayManager.drawBitmap(weatherImage, 16, 16, 9, 7);

    // Margin for temperature and IQA
    int margin = 10;

    // Temperature
    String tempStr = String(String((int)round(weatherManager->currentTemperature)));
    int16_t x1, y1;
    uint16_t w1, h1;
    int wTempUnit = 12;
    matrix->setFont(NULL);
    matrix->setTextSize(1);
    matrix->getTextBounds(tempStr, 0, 0, &x1, &y1, &w1, &h1);
    int16_t xTemp = matrix->width() - margin - (w1 + wTempUnit);
    int16_t yTemp = 11 - (h1 / 2) - y1;
    matrix->setCursor(xTemp, yTemp);
    matrix->setTextColor(displayManager.color565(255, 255, 255));
    matrix->print(tempStr);

    int x2 = matrix->getCursorX();
    int y2 = matrix->getCursorY();
    matrix->drawCircle(x2 + 3, y2, 1, displayManager.color565(255, 255, 255));
    matrix->setCursor(x2 + 6, y2);
    matrix->print("C");

    // AQI
    String aqiStr = String(weatherManager->currentAqi);
    int16_t x3, y3;
    uint16_t w3, h3;
    int wAqiUnit = 12;
    matrix->setFont(NULL);
    matrix->setTextSize(1);
    matrix->getTextBounds(aqiStr, 0, 0, &x3, &y3, &w3, &h3);
    int16_t xAqi = matrix->width() - margin - (w3 + wAqiUnit);
    int16_t yAqi = 21 - (h3 / 2) - y3;
    matrix->setCursor(xAqi, yAqi);
    matrix->setTextColor(weatherManager->getAqiColor(weatherManager->currentAqi));
    matrix->print(aqiStr);

    matrix->setFont(&Picopixel);
    matrix->setTextColor(displayManager.color565(255, 255, 255));
    matrix->setCursor(matrix->getCursorX() + 2, matrix->getCursorY());
    matrix->print("AQI");

    matrix->show();
    weatherManager->setDirty(false);
}

void CurrentWeatherScreen::update()
{
    if (weatherManager->isDirty())
    {
        draw();
    }
    weatherManager->update();
}
