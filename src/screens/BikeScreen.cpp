#include "BikeScreen.h"
#include "bitmap.h"
#include <math.h>
#include <Fonts/Picopixel.h>
#include "managers/DisplayManager.h"

BikeScreen::BikeScreen(Adafruit_Protomatter *m, BikeManager *w)
{
    matrix = m;
    bikeManager = w;
}

void BikeScreen::draw()
{
    matrix->fillScreen(0);
    // Station name
    matrix->setTextSize(1);
    matrix->setFont(&Picopixel);
    matrix->setTextColor(displayManager.color565(255, 255, 255));
    matrix->setCursor(2, 6);
    matrix->print(bikeManager->stationName);

    // Bike image
    displayManager.drawBitmap(bitmap_bike, 35, 19, 3, 11);

    // Bike number
    int16_t x1, y1;
    uint16_t w1, h1;

    displayManager.drawBitmap(bitmap_heart, 5, 5, 56, 11);
    String bikesStr = String(bikeManager->bikes);
    matrix->getTextBounds(bikesStr, 0, 0, &x1, &y1, &w1, &h1);
    matrix->setCursor(54 - w1, 15);
    matrix->print(bikesStr);

    displayManager.drawBitmap(bitmap_lightning, 5, 5, 56, 18);
    String eBikesStr = String(bikeManager->eBikes);
    matrix->getTextBounds(eBikesStr, 0, 0, &x1, &y1, &w1, &h1);
    matrix->setCursor(54 - w1, 22);
    matrix->print(eBikesStr);

    displayManager.drawBitmap(bitmap_parking, 5, 5, 56, 25);
    String availableStandsStr = String(bikeManager->availableStands);
    matrix->getTextBounds(availableStandsStr, 0, 0, &x1, &y1, &w1, &h1);
    matrix->setCursor(54 - w1, 29);
    matrix->print(availableStandsStr);

    matrix->show();
    bikeManager->setDirty(false);
}

void BikeScreen::update()
{
    if (bikeManager->isDirty())
    {
        draw();
    }
    bikeManager->update();
}
