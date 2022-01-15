#pragma once

#include <GLCore.h>
#include <reactphysics3d/reactphysics3d.h>

#include <memory>
#include <vector>

#include "lights/Lights.h"
#include "util/opengl/PerspectiveCamera.h"
#include "util/opengl/Shader.h"
#include "util/opengl/VertexArray.h"
#include "util/opengl/VertexBuffer.h"
#include <terrain/TerrainModel.h>

class Terrain {
 public:
  Terrain(glm::vec3 position, int numHorizontal, int numVertical,
          float mapWidth, float mapHeight, float noiseFreq, float noiseAmp);
  void generateModel();
  void freeModel();

  void update(GLCore::Timestep ts, float interpolationFactor = -1);
  void render(opengl::PerspectiveCamera& camera, lights::LightScene& lightScene,
              glm::vec2 goalPosition, float goalRadius);
  void imGuiRender(reactphysics3d::PhysicsWorld* physicsWorld,
                   reactphysics3d::PhysicsCommon& physicsCommon);

  void addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                  reactphysics3d::PhysicsCommon& physicsCommon);
  void removePhysics(reactphysics3d::PhysicsWorld* physicsWorld);

  glm::vec3 getPosition() { return position; }

  float getMinHeight() { return minHeight; }

 private:
  int numRows;
  int numCols;
  int numVertices;
  float mapWidth;
  float mapHeight;

  float noiseFreq;
  float noiseAmp;

  glm::vec3 position;
  glm::vec3 color;

  std::vector<float> heightMap;
  TerrainModel model;
  float minHeight;
  float maxHeight;

  opengl::Shader shader;

  reactphysics3d::RigidBody* rigidBody;
  reactphysics3d::Collider* collider;
  reactphysics3d::Transform prevTransform;
};
