#include "BallModel.h"

#include <glad/glad.h>

#include <memory>

#include "util/IndexBuffer.h"
#include "util/VertexArray.h"
#include "util/VertexBuffer.h"

BallModel::BallModel() {
  float radius = 1.0f;

  vertexData.clear();
  indexData.clear();

  for (int i = 0; i <= STACK_COUNT; i++) {
    float stackAngle = PI / 2 - i * STACK_STEP;
    float xy = radius * cosf(stackAngle);
    float z = radius * sinf(stackAngle);

    int k1 = i * (SECTOR_COUNT + 1);
    int k2 = k1 + SECTOR_COUNT + 1;

    for (int j = 0; j <= SECTOR_COUNT; j++, k1++, k2++) {
      float sectorAngle = j * SECTOR_STEP;

      float x = xy * cosf(sectorAngle);
      float y = xy * sinf(sectorAngle);

      // position
      vertexData.push_back(x);
      vertexData.push_back(y);
      vertexData.push_back(z);

      // normal
      vertexData.push_back(x / radius);
      vertexData.push_back(y / radius);
      vertexData.push_back(z / radius);

      // indices
      // 2 triangles per sector excluding first and last stacks
      // k1 => k2 => k1+1
      if (i != 0) {
        indexData.push_back(k1);
        indexData.push_back(k2);
        indexData.push_back(k1 + 1);
      }

      // k1+1 => k2 => k2+1
      if (i != (STACK_COUNT - 1)) {
        indexData.push_back(k1 + 1);
        indexData.push_back(k2);
        indexData.push_back(k2 + 1);
      }
    }
  }

  vertexArray = std::make_unique<opengl::VertexArray>();
  vertexArray->bind();

  vertexBuffer = std::make_unique<opengl::VertexBuffer>(
      vertexData.size() * sizeof(float), vertexData.data(), 6 * sizeof(float),
      GL_STATIC_DRAW);
  vertexBuffer->setVertexAttribute(0, 3, GL_FLOAT, 0);  // position
  vertexBuffer->setVertexAttribute(1, 3, GL_FLOAT,
                                   3 * sizeof(float));  // normal

  indexBuffer = std::make_unique<opengl::IndexBuffer>(
      indexData.size() * sizeof(unsigned int), indexData.data(),
      GL_STATIC_DRAW);
}

void BallModel::freeModel() {
  indexBuffer->free();
  vertexBuffer->free();
  vertexArray->free();
}
