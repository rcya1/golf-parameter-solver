#pragma once

#include "terrain/TerrainModel.h"

#include <GlCore.h>
#include <glm/glm.hpp>
#include <reactphysics3d/reactphysics3d.h>

#include <vector>


class Goal;
class TerrainRenderer;

class Terrain {
 public:
  Terrain(glm::vec3 position, int numHorizontal, int numVertical,
          float mapWidth, float mapHeight, float noiseFreq, float noiseAmp);
  void generateModel(Goal& goal);
  void freeModel();

  void update(GLCore::Timestep ts, float interpolationFactor = -1);
  void render(TerrainRenderer& renderer, glm::vec2 startPosition, float highlightRadius, glm::vec3 highlightColor);
  void imGuiRender(Goal& goal, reactphysics3d::PhysicsWorld* physicsWorld,
                   reactphysics3d::PhysicsCommon& physicsCommon);

  void addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                  reactphysics3d::PhysicsCommon& physicsCommon);
  void removePhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                     reactphysics3d::PhysicsCommon& physicsCommon);

  float getHeight(int col, int row) {
    return heightMap[row * (numCols + 1) + col];
  };

  glm::vec3 getPosition() { return position; }
  float getWidth() { return mapWidth; }
  float getHeight() { return mapHeight; }

  int getNumRows() { return numRows; }
  int getNumCols() { return numCols; }

  float getHSpacing() { return this->mapWidth / numCols; }
  float getVSpacing() { return this->mapHeight / numRows; }

  float getMinHeight() { return minHeight; }

  glm::vec2 convertUV(glm::vec2 uv) {
    return glm::vec2 {(uv.x - 0.5) * mapWidth + position.x,
                     (uv.y - 0.5) * mapHeight + position.z};
  }

  float getHeightFromRelative(glm::vec2 uv);

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
