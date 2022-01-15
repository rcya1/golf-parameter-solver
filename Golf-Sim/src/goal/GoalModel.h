#pragma once

#include <memory>
#include <vector>

#include "util/opengl/VertexArray.h"
#include "util/opengl/VertexBuffer.h"

class GoalModel {
 public:
  GoalModel();
  void generateModel();
  void freeModel();

  std::unique_ptr<opengl::VertexArray>& getVertexArray() { return vertexArray; }

  int getNumVertices() { return numVertices; }

 private:
  std::vector<float> vertices;
  int numVertices;

  std::unique_ptr<opengl::VertexArray> vertexArray;
  std::unique_ptr<opengl::VertexBuffer> vertexBuffer;
};
