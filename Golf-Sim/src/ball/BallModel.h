#pragma once

#include "../util/VertexArray.h"
#include "../util/VertexBuffer.h"
#include "../util/IndexBuffer.h"

#include <vector>
#include <memory>

#include <glad/glad.h>

class BallModel {
public:
  const float PI = 3.14159265f;

  const int SECTOR_COUNT = 36;
  const int STACK_COUNT = 18;

  const float SECTOR_STEP = 2 * PI / SECTOR_COUNT;
  const float STACK_STEP = PI / STACK_COUNT;

  static BallModel& getInstance() {
    static BallModel instance;
    return instance;
  }

  std::unique_ptr<VertexArray>& getVertexArray() {
    return vertexArray;
  }

  int getIndexDataSize() {
    return indexData.size();
  }

  void freeModel();
private:
  std::vector<float> vertexData;
  std::vector<unsigned int> indexData;

  std::unique_ptr<VertexArray> vertexArray;
  std::unique_ptr<VertexBuffer> vertexBuffer;
  std::unique_ptr<IndexBuffer> indexBuffer;

  BallModel();
public:
  BallModel(BallModel const&) = delete;
  void operator=(BallModel const&) = delete;
};