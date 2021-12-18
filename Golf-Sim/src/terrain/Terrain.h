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

class Terrain {
 public:
  Terrain(glm::vec3 position, int numHorizontal, int numVertical,
          float mapWidth, float mapHeight);
  void generateModel(float noiseFreq, float noiseAmp);
  void freeModel();

  void update(GLCore::Timestep ts, float interpolationFactor = -1);
  void render(opengl::PerspectiveCamera& camera, lights::LightScene& lightScene);
  void imGuiRender();

  void addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                  reactphysics3d::PhysicsCommon& physicsCommon);
  void removePhysics(reactphysics3d::PhysicsWorld* physicsWorld);

 private:
  int numRows;
  int numCols;
  float mapWidth;
  float mapHeight;

  glm::vec3 position;
  glm::vec3 color;

  std::vector<float> heightMap;
  float minHeight;
  float maxHeight;
  std::vector<float> vertices;

  std::unique_ptr<opengl::VertexArray> vertexArray;
  std::unique_ptr<opengl::VertexBuffer> vertexBuffer;

  opengl::Shader shader;

  std::pair<float, float> getXZ(int index);
  void addVertex(int index, std::vector<float>& norm);
  std::vector<float> getNormal(int i1, int i2, int i3);

  reactphysics3d::RigidBody* rigidBody;
  reactphysics3d::Collider* collider;
  reactphysics3d::Transform prevTransform;
};
