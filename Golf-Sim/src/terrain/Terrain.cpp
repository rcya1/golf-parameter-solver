#include "Terrain.h"

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
      numVertices(0),
      mapWidth(mapWidth),
      mapHeight(mapHeight),
      position(position),
      color(0.1f, 0.35f, 0.1f),
      minHeight(0.0),
      maxHeight(0.0),
      noiseFreq(noiseFreq),
      noiseAmp(noiseAmp),
      rigidBody(nullptr),
      collider(nullptr),
      shader("assets/shaders/TerrainVertexShader.vert",
             "assets/shaders/TerrainFragmentShader.frag") {
  generateModel();
}

void Terrain::generateModel() {
  if (heightMap.size() > 0) {
    freeModel();
  }

  noise::initNoise();

  heightMap = std::vector<float>((numRows + 1) * (numCols + 1));

  const float hSpacing = this->mapWidth / numCols;
  const float vSpacing = this->mapHeight / numRows;

  minHeight = -noiseAmp;
  maxHeight = noiseAmp;

  for (int j = 0; j <= numCols; j++) {
    for (int i = 0; i <= numRows; i++) {
      float x = j * hSpacing - this->mapWidth / 2;
      float z = i * vSpacing - this->mapHeight / 2;

      // float height = noiseAmp;
      // float height = (x * x + z * z) / 10.0;
      float height = noise::noise(x / noiseFreq, -5, z / noiseFreq) * noiseAmp;

      heightMap[i * (static_cast<long long>(numCols) + 1) + j] = height;
    }
  }

  model.generateModel(&heightMap, numCols, numRows, mapWidth, mapHeight);
}

void Terrain::freeModel() {
  heightMap.clear();

  model.freeModel();
}

void Terrain::update(GLCore::Timestep ts, float interpolationFactor) {
  if (this->rigidBody == nullptr || interpolationFactor == -1) {
    return;
  }
}

void Terrain::render(opengl::PerspectiveCamera& camera,
                     lights::LightScene& lightScene, glm::vec2 goalPosition,
                     float goalRadius) {
  shader.activate();
  shader.setVec3f("material.ambient", 0.0f, 0.0f, 0.0f);
  shader.setVec3f("material.diffuse", color);
  shader.setVec3f("material.specular", 0.025f, 0.025f, 0.025f);
  shader.setFloat("material.shininess", 2);
  shader.setVec3f("viewPos", camera.getPos());
  shader.setMat4f("view", false, camera.getViewMatrix());
  shader.setMat4f("projection", false, camera.getProjectionMatrix());
  shader.setVec2f("goalPos", goalPosition);
  shader.setFloat("goalRadius", goalRadius);

  lights::setLightScene(shader, lightScene);
  model.getVertexArray()->bind();

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, position);
  shader.setMat4f("model", false, glm::value_ptr(model));
  glDrawArrays(GL_TRIANGLES, 0, numVertices);
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
    removePhysics(physicsWorld);
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
  reactphysics3d::HeightFieldShape* heightFieldShape =
      physicsCommon.createHeightFieldShape(
          numCols + 1, numRows + 1, minHeight, maxHeight, heightMap.data(),
          reactphysics3d::HeightFieldShape::HeightDataType::HEIGHT_FLOAT_TYPE,
          1, 1.0f, scaling);
  reactphysics3d::Transform shapeTransform =
      reactphysics3d::Transform::identity();

  this->collider =
      this->rigidBody->addCollider(heightFieldShape, shapeTransform);
  this->collider->setCollisionCategoryBits(CollisionCategory::TERRAIN);
  this->collider->setCollideWithMaskBits(CollisionCategory::BALL);
}

void Terrain::removePhysics(reactphysics3d::PhysicsWorld* physicsWorld) {
  physicsWorld->destroyRigidBody(this->rigidBody);
}
