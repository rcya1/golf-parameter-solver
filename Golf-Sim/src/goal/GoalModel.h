#pragma once

#include <memory>
#include <vector>

#include "util/opengl/VertexArray.h"
#include "util/opengl/VertexBuffer.h"
#include <terrain/Terrain.h>

class GoalModel {
 public:
  const float PI = 3.14159265f;

  const int SECTOR_COUNT = 24; // divisible by 4 to ensure cardinal points are included
  const float SECTOR_STEP = 2 * PI / SECTOR_COUNT;
  const float GOAL_HEIGHT = 3.0;

  GoalModel();
  void generateModel(Terrain& terrain, glm::vec2 relativeCoords, float radius);
  void freeModel();

  std::unique_ptr<opengl::VertexArray>& getVertexArray() { return vertexArray; }
  glm::vec3 getPosition() { return pos; }

  int getNumVertices() { return numVertices; }

 private:
  glm::vec3 pos;
  std::vector<float> vertices;
  int numVertices;

  std::unique_ptr<opengl::VertexArray> vertexArray;
  std::unique_ptr<opengl::VertexBuffer> vertexBuffer;

  void addVertex(glm::vec3 a, glm::vec3 norm);
  glm::vec3 getNormal(glm::vec3 a, glm::vec3 b, glm::vec3 c);
  void addTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);
};

struct GoalModelPoint {
  glm::vec2 pos;
  int row;
  int col;
  float height;
  std::vector<glm::vec3> cellPoints;
};
