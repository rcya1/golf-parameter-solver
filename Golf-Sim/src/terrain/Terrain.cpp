#include "Terrain.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>

#include "PerlinNoise.h"
#include "lights/Lights.h"
#include "util/opengl/PerspectiveCamera.h"

Terrain::Terrain(glm::vec3 position, int numCols, int numRows, float mapWidth,
                 float mapHeight)
    : numCols(numCols),
      numRows(numRows),
      mapWidth(mapWidth),
      mapHeight(mapHeight),
      position(position),
      color(0.1f, 0.35f, 0.1f),
      minHeight(0.0),
      maxHeight(0.0),
      rigidBody(nullptr),
      collider(nullptr),
      shader("assets/shaders/LightingVertexShader.vert",
             "assets/shaders/LightingFragmentShader.frag") {}

void Terrain::generateModel(float noiseFreq, float noiseAmp) {
  if (heightMap.size() > 0) {
    freeModel();
  }

  heightMap = std::vector<float>((numRows + 1) * (numCols + 1));

  const float hSpacing = this->mapWidth / numCols;
  const float vSpacing = this->mapHeight / numRows;

  minHeight = -noiseAmp;
  maxHeight = noiseAmp;

  for (int j = 0; j <= numCols; j++) {
    for (int i = 0; i <= numRows; i++) {
      float x = i * hSpacing - this->mapWidth / 2;
      float z = j * vSpacing - this->mapHeight / 2;

      // float height = noiseAmp;
      // float height = (x * x + z * z) / 10.0;
      float height = noise::noise(x / noiseFreq, -5, z / noiseFreq) * noiseAmp;

      heightMap[j * (static_cast<long long>(numCols) + 1) + i] = height;
    }
  }

  for (int j = 0; j < numCols; j++) {
    for (int i = 0; i < numRows; i++) {
      int topLeft = j * (numCols + 1) + i;
      int topRight = (j + 1) * (numCols + 1) + i;
      int botLeft = j * (numCols + 1) + i + 1;
      int botRight = (j + 1) * (numCols + 1) + i + 1;

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
}

void Terrain::freeModel() {
  heightMap.clear();
  vertices.clear();

  vertexArray->free();
  vertexBuffer->free();
}

void Terrain::update(GLCore::Timestep ts, float interpolationFactor) {
  if (this->rigidBody == nullptr || interpolationFactor == -1) {
    return;
  }
}

void Terrain::render(opengl::PerspectiveCamera& camera,
                     lights::LightScene& lightScene) {
  shader.activate();
  shader.setVec3f("material.ambient", 0.0f, 0.0f, 0.0f);
  shader.setVec3f("material.diffuse", color);
  shader.setVec3f("material.specular", 0.025f, 0.025f, 0.025f);
  shader.setFloat("material.shininess", 2);
  shader.setVec3f("viewPos", camera.getPos());
  shader.setMat4f("view", false, camera.getViewMatrix());
  shader.setMat4f("projection", false, camera.getProjectionMatrix());

  lights::setLightScene(shader, lightScene);
  vertexArray->bind();

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, position);
  shader.setMat4f("model", false, glm::value_ptr(model));
  glDrawArrays(GL_TRIANGLES, 0, 2 * 3 * numRows * numCols);
}

void Terrain::imGuiRender() {
  ImGui::Begin("Terrain Controls");
  ImGui::SetWindowFontScale(2.0);
  if (ImGui::DragFloat3("Position", glm::value_ptr(position), 1.0f, -10.0f,
                        10.0f)) {
    reactphysics3d::Vector3 position(position.x, position.y, position.z);
    reactphysics3d::Quaternion orientation =
        reactphysics3d::Quaternion::identity();
    reactphysics3d::Transform newTransform(position, orientation);

    rigidBody->setTransform(newTransform);
  }
  ImGui::ColorEdit3("Color", glm::value_ptr(color));
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
}

void Terrain::removePhysics(reactphysics3d::PhysicsWorld* physicsWorld) {
  physicsWorld->destroyRigidBody(this->rigidBody);
}

std::pair<float, float> Terrain::getXZ(int index) {
  int j = index / (numCols + 1);
  int i = index % (numCols + 1);

  const float hSpacing = this->mapWidth / numCols;
  const float vSpacing = this->mapHeight / numRows;

  return std::make_pair(i * hSpacing - this->mapWidth / 2,
                        j * vSpacing - this->mapHeight / 2);
}

void Terrain::addVertex(int index, std::vector<float>& norm) {
  std::pair<float, float> xz = getXZ(index);
  float x = xz.first;
  float y = heightMap[index];
  float z = xz.second;

  vertices.push_back(x);
  vertices.push_back(y);
  vertices.push_back(z);

  vertices.push_back(norm[0]);
  vertices.push_back(norm[1]);
  vertices.push_back(norm[2]);
}

// returns the upward facing normal (one with positive y coordinate)
std::vector<float> Terrain::getNormal(int i1, int i2, int i3) {
  std::pair<float, float> xz1 = getXZ(i1);
  std::pair<float, float> xz2 = getXZ(i2);
  std::pair<float, float> xz3 = getXZ(i3);

  glm::vec3 a(xz1.first - xz2.first, heightMap[i1] - heightMap[i2],
              xz1.second - xz2.second);
  glm::vec3 b(xz3.first - xz2.first, heightMap[i3] - heightMap[i2],
              xz3.second - xz2.second);

  glm::vec3 c = glm::normalize(glm::cross(a, b));

  if (c.y < 0) c *= -1;

  return std::vector<float>{c.x, c.y, c.z};
}
