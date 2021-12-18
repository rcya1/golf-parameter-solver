#pragma once

#include <glad/glad.h>

#include <memory>
#include <vector>

#include "util/opengl/IndexBuffer.h"
#include "util/opengl/VertexArray.h"
#include "util/opengl/VertexBuffer.h"

class BallModel {
 public:
  const float PI = 3.14159265f;

  const int SECTOR_COUNT = 18;
  const int STACK_COUNT = 9;

  const float SECTOR_STEP = 2 * PI / SECTOR_COUNT;
  const float STACK_STEP = PI / STACK_COUNT;

  static BallModel& getInstance() {
    static BallModel instance;
    return instance;
  }

  std::unique_ptr<opengl::VertexArray>& getVertexArray() { return vertexArray; }

  int getIndexDataSize() { return indexData.size(); }

  void freeModel();

 private:
  std::vector<float> vertexData;
  std::vector<unsigned int> indexData;

  std::unique_ptr<opengl::VertexArray> vertexArray;
  std::unique_ptr<opengl::VertexBuffer> vertexBuffer;
  std::unique_ptr<opengl::IndexBuffer> indexBuffer;

  BallModel();

 public:
  BallModel(BallModel const&) = delete;
  void operator=(BallModel const&) = delete;
};