#include "TransportScreen.h"
#include "bitmap.h"
#include <Fonts/Picopixel.h>
#include "managers/DisplayManager.h"

TransportScreen::TransportScreen(Adafruit_Protomatter *m, TransportManager *w)
{
    matrix = m;
    transportManager = w;
}

void TransportScreen::draw()
{
    matrix->fillScreen(0);
    displayManager.drawBitmap(bitmap_line_a, 11, 11, 2, 2);
    matrix->drawLine(0, 16, 64, 16, displayManager.color565(30, 30, 30));
    displayManager.drawBitmap(bitmap_line_a, 11, 11, 2, 19);

    matrix->setTextSize(1);
    matrix->setFont(&Picopixel);
    matrix->setTextColor(displayManager.color565(255, 255, 255));

    matrix->setCursor(15, 6);
    String direction1 = String(transportManager->direction1);
    if (transportManager->isPerturbations)
    {
        matrix->setTextColor(displayManager.color565(255, 0, 0));
        direction1 = "! " + direction1;
    }
    if (direction1.length() > 10)
    {
        direction1 = direction1.substring(0, 10) + "...";
    }
    matrix->print(direction1);

    matrix->setTextColor(displayManager.color565(255, 255, 255));
    matrix->setCursor(15, 23);
    String direction2 = String(transportManager->direction2);
    if (transportManager->isPerturbations)
    {
        matrix->setTextColor(displayManager.color565(255, 0, 0));
        direction2 = "! " + direction2;
    }
    if (direction2.length() > 10)
    {
        direction2 = direction2.substring(0, 10) + "...";
    }
    matrix->print(direction2);

    matrix->setTextColor(displayManager.color565(255, 255, 255));
    matrix->setCursor(15, 12);
    String times1 = "";
    if (transportManager->next1[0] != "")
    {
        times1 = String(transportManager->next1[0]);
        if (transportManager->next1[1] != "")
        {
            times1 += String(", ") + String(transportManager->next1[1]);
        }
    }
    matrix->print(times1);

    matrix->setCursor(15, 29);
    String times2 = "";
    if (transportManager->next2[0] != "")
    {
        times2 = String(transportManager->next2[0]);
        if (transportManager->next2[1] != "")
        {
            times2 += String(", ") + String(transportManager->next2[1]);
        }
    }
    matrix->print(times2);

    matrix->show();
    transportManager->setDirty(false);
}

void TransportScreen::update()
{
    if (transportManager->isDirty())
    {
        draw();
    }
    transportManager->update();
}
