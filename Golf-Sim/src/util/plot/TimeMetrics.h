#pragma once

#include <GLCore.h>

#include "util/plot/Buffers.h"

class TimeMetrics {
 public:
  void update(GLCore::Timestep& ts);
  void imGuiRender(float dpiScale);

 private:
  ScrollingBuffer timePerFrameScrollBuffer;
  RollingBuffer timePerFrameRollBuffer;
  float time = 0;
  float historyLength = 10.0f;
};
