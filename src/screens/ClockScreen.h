#pragma once

#include <Adafruit_Protomatter.h>
#include "../managers/TimeManager.h"
#include "Screen.h"

class ClockScreen : public Screen
{
public:
  ClockScreen(Adafruit_Protomatter *matrix, TimeManager *timeManager);
  void draw() override;
  void update() override;

private:
  Adafruit_Protomatter *matrix;
  TimeManager *timeManager;
  bool colonVisible = true;
  unsigned long lastBlink = 0;
};
