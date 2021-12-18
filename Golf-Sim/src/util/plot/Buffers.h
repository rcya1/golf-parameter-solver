#pragma once

#include "imgui.h"

struct ScrollingBuffer {
  int maxSize;
  int offset;
  ImVector<ImVec2> data;
  ScrollingBuffer(int max_size = 2000) {
    maxSize = max_size;
    offset = 0;
    data.reserve(maxSize);
  }
  void addPoint(float x, float y) {
    if (data.size() < maxSize)
      data.push_back(ImVec2(x, y));
    else {
      data[offset] = ImVec2(x, y);
      offset = (offset + 1) % maxSize;
    }
  }
  void erase() {
    if (data.size() > 0) {
      data.shrink(0);
      offset = 0;
    }
  }
};

struct RollingBuffer {
  float span;
  ImVector<ImVec2> data;
  RollingBuffer() {
    span = 10.0f;
    data.reserve(2000);
  }
  void addPoint(float x, float y) {
    float xmod = fmodf(x, span);
    if (!data.empty() && xmod < data.back().x) data.shrink(0);
    data.push_back(ImVec2(xmod, y));
  }
};
