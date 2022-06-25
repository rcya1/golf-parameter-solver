#include "TerrainModel.h"

#include "util/opengl/VertexArray.h"
#include "util/opengl/VertexBuffer.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <memory>

TerrainModel::TerrainModel() {}

void TerrainModel::generateModel(std::vector<float>* heightMap, int numCols,
                                 int numRows, float mapWidth, float mapHeight,
                                 glm::vec2 goalPos, float goalRadius) {
  this->heightMap = heightMap;
  this->numCols = numCols;
  this->numRows = numRows;
  this->mapWidth = mapWidth;
  this->mapHeight = mapHeight;

  freeModel();

  float gx = goalRadius + goalPos.x * (mapWidth - 2 * goalRadius);
  float gy = goalRadius + goalPos.y * (mapHeight- 2 * goalRadius);

  float l = gx - goalRadius;
  float r = gx + goalRadius;
  float b = gy - goalRadius;
  float t = gy + goalRadius;

  float hSpacing = mapWidth / numCols;
  float vSpacing = mapHeight / numRows;

  // generate bounding box
  int bl = static_cast<int>(floorf(l / hSpacing));
  int br = static_cast<int>(ceilf(r / hSpacing));
  int bb = static_cast<int>(floorf(b / vSpacing));
  int bt = static_cast<int>(ceilf(t / vSpacing));

  //printf("%d %d %d %d", bl, br, bb, bt);

  for (int j = 0; j < numCols; j++) {
    for (int i = 0; i < numRows; i++) {
      if (j >= bl && j < br && i >= bb && i < bt) {
        continue;
      }

      int topLeft = i * (numCols + 1) + j;
      int topRight = (i + 1) * (numCols + 1) + j;
      int botLeft = i * (numCols + 1) + j + 1;
      int botRight = (i + 1) * (numCols + 1) + j + 1;

      std::vector<float> n1 = getNormal(topLeft, topRight, botLeft);
      addVertex(topLeft, n1);
      addVertex(topRight, n1);
      addVertex(botLeft, n1);

      std::vector<float> n2 = getNormal(botLeft, topRight, botRight);
      addVertex(botLeft, n2);
      addVertex(topRight, n2);
      addVertex(botRight, n2);
    }
  }

  vertexArray = std::make_unique<opengl::VertexArray>();
  vertexArray->bind();

  vertexBuffer = std::make_unique<opengl::VertexBuffer>(
      vertices.size() * sizeof(float), vertices.data(), 6 * sizeof(float),
      GL_STATIC_DRAW);
  vertexBuffer->setVertexAttribute(0, 3, GL_FLOAT, 0);
  vertexBuffer->setVertexAttribute(1, 3, GL_FLOAT, 3 * sizeof(float));

  vertexArray->unbind();

  numVertices = 2 * 3 * numRows * numCols;
}

void TerrainModel::freeModel() {
  if (vertices.empty()) return;

  vertices.clear();

  vertexArray->free();
  vertexBuffer->free();
}

std::pair<float, float> TerrainModel::getXZ(int index) {
  int j = index / (numCols + 1);
  int i = index % (numCols + 1);

  const float hSpacing = this->mapWidth / numCols;
  const float vSpacing = this->mapHeight / numRows;

  return std::make_pair(i * hSpacing - this->mapWidth / 2,
                        j * vSpacing - this->mapHeight / 2);
}

void TerrainModel::addVertex(int index, std::vector<float>& norm) {
  std::pair<float, float> xz = getXZ(index);
  float x = xz.first;
  float y = (*heightMap)[index];
  float z = xz.second;

  vertices.push_back(x);
  vertices.push_back(y);
  vertices.push_back(z);

  vertices.push_back(norm[0]);
  vertices.push_back(norm[1]);
  vertices.push_back(norm[2]);
}

// returns the upward facing normal (one with positive y coordinate)
std::vector<float> TerrainModel::getNormal(int i1, int i2, int i3) {
  std::pair<float, float> xz1 = getXZ(i1);
  std::pair<float, float> xz2 = getXZ(i2);
  std::pair<float, float> xz3 = getXZ(i3);

  glm::vec3 a(xz1.first - xz2.first, (*heightMap)[i1] - (*heightMap)[i2],
              xz1.second - xz2.second);
  glm::vec3 b(xz3.first - xz2.first, (*heightMap)[i3] - (*heightMap)[i2],
              xz3.second - xz2.second);

  glm::vec3 c = glm::normalize(glm::cross(a, b));

  if (c.y < 0) c *= -1;

  return std::vector<float>{c.x, c.y, c.z};
}
