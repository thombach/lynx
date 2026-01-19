#pragma once

class Screen
{
public:
  virtual ~Screen() = default;
  virtual void draw() = 0;
  virtual void update() {}
};