#include "Terrain.h"

#include "goal/Goal.h"
#include "terrain/TerrainModel.h"
#include "terrain/TerrainRenderer.h"

#include "util/CollisionCategory.h"

#include "PerlinNoise.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

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
      collider(nullptr) {}

void Terrain::generateModel(Goal& goal) {
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

  // std::cout << std::fixed;
  // std::cout << std::setprecision(3);
  // for (int i = 0; i <= numRows; i++) {
  //   for(int j = 0; j <= numCols; j++) {
  //     float val = heightMap[i * (static_cast<long long>(numCols) + 1) +
  //                           j];
  //     if (val >= 0) std::cout << " ";
  //     std::cout << val << " ";
  //   }
  //   std::cout << std::endl;
  // }

  terrainModel.generateModel(&heightMap, numCols, numRows, mapWidth,
                             mapHeight, goal.getRelativePosition(), goal.getRadius());
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

void Terrain::render(TerrainRenderer& renderer, glm::vec2 startPosition, float highlightRadius, glm::vec3 highlightColor) {
  glm::vec2 startPos = convertUV(startPosition);
  renderer.add(TerrainRenderJob{terrainModel, position, color, startPos, highlightRadius, highlightColor});
}

void Terrain::imGuiRender(Goal& goal,
                          reactphysics3d::PhysicsWorld* physicsWorld,
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
    generateModel(goal);
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

  rigidBody = nullptr;
  shape = nullptr;
}

// finds the straight down projection of p onto the plane formed by a, b, c
float projectToPlane(glm::vec2 p, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
  glm::vec3 A = b - a;
  glm::vec3 B = c - a;
  glm::vec3 C = glm::cross(A, B);
  return (C.x * (a.x - p.x) + C.z * (a.z - p.y) + C.y * a.y) / C.y;
}

void printVec3(glm::vec3 vec) {
  std::cout << vec.x << " : " << vec.y << " : " << vec.z << std::endl;
}

float Terrain::getHeightFromUV(glm::vec2 uv) {
  float x = uv.x;
  float y = uv.y;

  float hSpacing = getHSpacing();
  float vSpacing = getVSpacing();

  int col = x / hSpacing;
  int row = y / vSpacing;

  float ax = x - hSpacing * col;
  float ay = y - vSpacing * row;

  bool isTopRight = ay * hSpacing > -vSpacing * ax + vSpacing * hSpacing;
  // std::cout << x << " : " << y << " : " << col << " : " << row << " : " <<
  // isTopRight << std::endl;

  float tl = col * hSpacing;
  float tr = (col + 1) * hSpacing;
  float tb = row * vSpacing;
  float tt = (row + 1) * vSpacing;
  glm::vec3 topLeft = glm::vec3(tl, getHeight(col, row + 1), tt);
  glm::vec3 topRight = glm::vec3(tr, getHeight(col + 1, row + 1), tt);
  glm::vec3 botLeft = glm::vec3(tl, getHeight(col, row), tb);
  glm::vec3 botRight = glm::vec3(tr, getHeight(col + 1, row), tb);
  float height = isTopRight ? projectToPlane(uv, topLeft, topRight, botRight)
                            : projectToPlane(uv, topLeft, botRight, botLeft);

  return height;
}
