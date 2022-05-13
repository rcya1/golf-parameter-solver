#include "Terrain.h"

#include <terrain/TerrainRenderer.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>

#include "PerlinNoise.h"
#include "lights/Lights.h"
#include "util/CollisionCategory.h"
#include "util/opengl/PerspectiveCamera.h"

Terrain::Terrain(glm::vec3 position, int numCols, int numRows, float mapWidth,
                 float mapHeight, float noiseFreq, float noiseAmp)
    : numCols(numCols),
      numRows(numRows),
      mapWidth(mapWidth),
      mapHeight(mapHeight),
      position(position),
      color(0.1f, 0.35f, 0.1f),
      minHeight(0.0),
      maxHeight(0.0),
      noiseFreq(noiseFreq),
      noiseAmp(noiseAmp),
      rigidBody(nullptr),
      collider(nullptr) {
  generateModel();
}

void Terrain::generateModel() {
  if (heightMap.size() > 0) {
    freeModel();
  }

  noise::initNoise();

  heightMap = std::vector<float>((numRows + 1) * (numCols + 1));

  minHeight = -noiseAmp;
  maxHeight = noiseAmp;

  for (int j = 0; j <= numCols; j++) {
    for (int i = 0; i <= numRows; i++) {
      float x = j * getHSpacing() - this->mapWidth / 2;
      float z = i * getVSpacing() - this->mapHeight / 2;

      // float height = noiseAmp;
      // float height = (x * x + z * z) / 10.0;
      float height = noise::noise(x / noiseFreq, -5, z / noiseFreq) * noiseAmp;

      heightMap[i * (static_cast<long long>(numCols) + 1) + j] = height;
    }
  }

  //std::cout << std::fixed;
  //std::cout << std::setprecision(3);
  //for (int i = 0; i <= numRows; i++) {
  //  for(int j = 0; j <= numCols; j++) {
  //    float val = heightMap[i * (static_cast<long long>(numCols) + 1) +
  //                          j];
  //    if (val >= 0) std::cout << " ";
  //    std::cout << val << " ";
  //  }
  //  std::cout << std::endl;
  //}

  terrainModel.generateModel(&heightMap, numCols, numRows, mapWidth, mapHeight);
}

void Terrain::freeModel() {
  heightMap.clear();
  terrainModel.freeModel();
}

void Terrain::update(GLCore::Timestep ts, float interpolationFactor) {
  if (this->rigidBody == nullptr || interpolationFactor == -1) {
    return;
  }
}

void Terrain::render(TerrainRenderer& renderer, glm::vec2 goalPos,
                     float goalRadius) {
  float grx = goalPos.x - position.x + mapWidth / 2.0;
  float gry = goalPos.y - position.z + mapHeight / 2.0;

  float l = grx - goalRadius;
  float r = grx + goalRadius;
  float b = gry - goalRadius;
  float t = gry + goalRadius;

  // generate bounding box
  float rl = static_cast<int>(floorf(l / getHSpacing())) * getHSpacing() + 
             position.x - mapWidth / 2.0;
  float rr = static_cast<int>(ceilf(r / getHSpacing())) * getHSpacing() +
             position.x - mapWidth / 2.0;
  float rb = static_cast<int>(floorf(b / getVSpacing())) * getVSpacing() +
             position.z - mapHeight / 2.0;
  float rt = static_cast<int>(ceilf(t / getVSpacing())) * getVSpacing() +
             position.z - mapHeight / 2.0;
  renderer.add(TerrainRenderJob{terrainModel, position, color, rl, rr, rb, rt});
}

void Terrain::imGuiRender(reactphysics3d::PhysicsWorld* physicsWorld,
                          reactphysics3d::PhysicsCommon& physicsCommon) {
  ImGui::Begin("Terrain Controls");
  if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.5f, -10.0f,
                        10.0f)) {
    reactphysics3d::Vector3 position(position.x, position.y, position.z);
    reactphysics3d::Quaternion orientation =
        reactphysics3d::Quaternion::identity();
    reactphysics3d::Transform newTransform(position, orientation);

    rigidBody->setTransform(newTransform);
  }
  ImGui::ColorEdit3("Color", glm::value_ptr(color));

  ImGui::DragFloat("Map Width", &mapWidth, 1.0, 1, 100);
  ImGui::DragFloat("Map Height", &mapHeight, 1.0, 1, 100);
  ImGui::DragInt("Number of Columns", &numCols, 1.0, 1, 100);
  ImGui::DragInt("Number of Rows", &numRows, 1.0, 1, 100);
  ImGui::DragFloat("Noise Frequency", &noiseFreq, 0.5f, 0.01f, 20.0f);
  ImGui::DragFloat("Noise Amp", &noiseAmp, 0.5f, 0.0f, 20.0f);
  if (ImGui::Button("Regenerate Terrain")) {
    generateModel();
    removePhysics(physicsWorld, physicsCommon);
    addPhysics(physicsWorld, physicsCommon);
  }
  ImGui::End();
}

void Terrain::addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                         reactphysics3d::PhysicsCommon& physicsCommon) {
  reactphysics3d::Vector3 position(position.x, position.y, position.z);
  reactphysics3d::Quaternion orientation =
      reactphysics3d::Quaternion::identity();
  reactphysics3d::Transform transform(position, orientation);

  this->rigidBody = physicsWorld->createRigidBody(transform);
  this->rigidBody->setType(reactphysics3d::BodyType::STATIC);

  reactphysics3d::Vector3 scaling(mapWidth / numCols, 1.0, mapHeight / numRows);
  this->shape = physicsCommon.createHeightFieldShape(
      numCols + 1, numRows + 1, minHeight, maxHeight, heightMap.data(),
      reactphysics3d::HeightFieldShape::HeightDataType::HEIGHT_FLOAT_TYPE, 1,
      1.0f, scaling);
  reactphysics3d::Transform shapeTransform =
      reactphysics3d::Transform::identity();

  this->collider = this->rigidBody->addCollider(shape, shapeTransform);
  this->collider->setCollisionCategoryBits(CollisionCategory::TERRAIN);
  this->collider->setCollideWithMaskBits(CollisionCategory::BALL);
}

void Terrain::removePhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                            reactphysics3d::PhysicsCommon& physicsCommon) {
  physicsWorld->destroyRigidBody(rigidBody);
  physicsCommon.destroyHeightFieldShape(shape);
}
