#pragma once

#include <GLCore.h>
#include <reactphysics3d/reactphysics3d.h>
#include <terrain/TerrainModel.h>

#include <memory>
#include <vector>

#include "terrain/TerrainRenderer.h"

#include "lights/Lights.h"
#include "util/opengl/PerspectiveCamera.h"
#include "util/opengl/Shader.h"
#include "util/opengl/VertexArray.h"
#include "util/opengl/VertexBuffer.h"

class Terrain {
 public:
  Terrain(glm::vec3 position, int numHorizontal, int numVertical,
          float mapWidth, float mapHeight, float noiseFreq, float noiseAmp);
  void generateModel();
  void freeModel();

  void update(GLCore::Timestep ts, float interpolationFactor = -1);
  void render(TerrainRenderer& renderer, glm::vec2 goalPos, float goalRadius);
  void imGuiRender(reactphysics3d::PhysicsWorld* physicsWorld,
                   reactphysics3d::PhysicsCommon& physicsCommon);

  void addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                  reactphysics3d::PhysicsCommon& physicsCommon);
  void removePhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                     reactphysics3d::PhysicsCommon& physicsCommon);

  float getHeight(int col, int row) { return heightMap[row * (numRows + 1) + col]; };

  glm::vec3 getPosition() { return position; }
  float getWidth() { return mapWidth; }
  float getHeight() { return mapHeight; }

  int getNumRows() { return numRows; }
  int getNumCols() { return numCols; }

  float getHSpacing() { return this->mapWidth / numCols; }
  float getVSpacing() { return this->mapHeight / numRows; }

  float getMinHeight() { return minHeight; }

 private:
  int numRows;
  int numCols;
  float mapWidth;
  float mapHeight;

  float noiseFreq;
  float noiseAmp;

  glm::vec3 position;
  glm::vec3 color;

  std::vector<float> heightMap;
  TerrainModel terrainModel;
  float minHeight;
  float maxHeight;

  reactphysics3d::RigidBody* rigidBody;
  reactphysics3d::Collider* collider;
  reactphysics3d::HeightFieldShape* shape;
  reactphysics3d::Transform prevTransform;
};
