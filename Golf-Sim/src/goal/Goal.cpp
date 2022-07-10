#include "Goal.h"

#include <reactphysics3d/collision/TriangleMesh.h>
#include <reactphysics3d/collision/TriangleVertexArray.h>
#include <reactphysics3d/collision/shapes/ConvexMeshShape.h>

#include "goal/GoalRenderer.h"
#include "terrain/Terrain.h"
#include "util/CollisionCategory.h"
#include "ImGuiConstants.h"

Goal::Goal(float x, float z, float r)
    : relativePosition(x, z), radius(r), color(0.1f, 0.35f, 0.1f) {}

void Goal::generateModel(Terrain& terrain) {
  freeModel();

  glm::vec2 relativeCoords = glm::vec2(
      relativePosition.x * (terrain.getWidth() - 2 * radius) + radius,
      relativePosition.y * (terrain.getHeight() - 2 * radius) + radius);

  goalModel.generateModel(terrain, relativeCoords, radius);
}

void Goal::freeModel() { goalModel.freeModel(); }

void Goal::render(GoalRenderer& renderer) {
  renderer.add(GoalRenderJob{goalModel, color});
}

void Goal::imGuiRender(reactphysics3d::PhysicsWorld* physicsWorld,
                       reactphysics3d::PhysicsCommon& physicsCommon,
                       Terrain& terrain) {
  ImGui::DragFloat2("Position", glm::value_ptr(relativePosition), 0.01f, 0.0f,
                    1.0f);
  ImGui::DragFloat("Radius", &radius, 0.1f, 0.1f, 5.0f);
  ImGui::ColorEdit3("Color", glm::value_ptr(color));

  setupGreenButton();
  if (ImGui::Button("Regenerate Goal")) {
    generateModel(terrain);
    removePhysics(physicsWorld, physicsCommon);
    addPhysics(physicsWorld, physicsCommon);
  }
  clearButtonStyle();
}

void Goal::addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                      reactphysics3d::PhysicsCommon& physicsCommon) {
  reactphysics3d::Vector3 position(goalModel.getPosition().x,
                                   goalModel.getPosition().y,
                                   goalModel.getPosition().z);
  reactphysics3d::Quaternion orientation =
      reactphysics3d::Quaternion::identity();
  reactphysics3d::Transform transform(position, orientation);

  this->rigidBody = physicsWorld->createRigidBody(transform);
  this->rigidBody->setType(reactphysics3d::BodyType::STATIC);

  terrainVA = new reactphysics3d::TriangleVertexArray(
      goalModel.getVerticesArray(GoalModelPart::TERRAIN_PART).size() / 3,
      goalModel.getVerticesArray(GoalModelPart::TERRAIN_PART).data(),
      3 * sizeof(float),
      goalModel.getIndicesArray(GoalModelPart::TERRAIN_PART).size() / 3,
      goalModel.getIndicesArray(GoalModelPart::TERRAIN_PART).data(),
      3 * sizeof(unsigned int),
      reactphysics3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
      reactphysics3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

  wallsVA = new reactphysics3d::TriangleVertexArray(
      goalModel.getVerticesArray(GoalModelPart::WALLS_PART).size() / 3,
      goalModel.getVerticesArray(GoalModelPart::WALLS_PART).data(),
      3 * sizeof(float),
      goalModel.getIndicesArray(GoalModelPart::WALLS_PART).size() / 3,
      goalModel.getIndicesArray(GoalModelPart::WALLS_PART).data(),
      3 * sizeof(unsigned int),
      reactphysics3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
      reactphysics3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

  bottomVA = new reactphysics3d::TriangleVertexArray(
      goalModel.getVerticesArray(GoalModelPart::BOTTOM_PART).size() / 3,
      goalModel.getVerticesArray(GoalModelPart::BOTTOM_PART).data(),
      3 * sizeof(float),
      goalModel.getIndicesArray(GoalModelPart::BOTTOM_PART).size() / 3,
      goalModel.getIndicesArray(GoalModelPart::BOTTOM_PART).data(),
      3 * sizeof(unsigned int),
      reactphysics3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
      reactphysics3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

  reactphysics3d::Transform shapeTransform =
      reactphysics3d::Transform::identity();

  // std::cout << goalModel.getVertices().size() << " : "
  //           << goalModel.getIndices().size() << std::endl;

  // for (auto x : goalModel.getVertices()) {
  //   std::cout << x << std::endl;
  // }

  // std::cout << std::endl;
  // for (auto x : goalModel.getIndices()) {
  //   std::cout << x << std::endl;
  // }

  triangleMesh = physicsCommon.createTriangleMesh();
  triangleMesh->addSubpart(terrainVA);
  triangleMesh->addSubpart(wallsVA);
  triangleMesh->addSubpart(bottomVA);

  shape = physicsCommon.createConcaveMeshShape(triangleMesh);
  this->collider = this->rigidBody->addCollider(shape, shapeTransform);
  this->collider->setCollisionCategoryBits(CollisionCategory::GOAL);
  this->collider->setCollideWithMaskBits(CollisionCategory::BALL);
}

void Goal::removePhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                         reactphysics3d::PhysicsCommon& physicsCommon) {
  physicsWorld->destroyRigidBody(rigidBody);
  physicsCommon.destroyConcaveMeshShape(shape);
  physicsCommon.destroyTriangleMesh(triangleMesh);

  delete terrainVA;
  delete wallsVA;
  delete bottomVA;

  rigidBody = nullptr;
  shape = nullptr;
  triangleMesh = nullptr;
  terrainVA = nullptr;
  wallsVA = nullptr;
  bottomVA = nullptr;
}

glm::vec2 Goal::getAbsolutePosition(Terrain& terrain) {
  return getAbsolutePosition(terrain.getPosition(), terrain.getWidth(),
                             terrain.getHeight());
}

glm::vec2 Goal::getAbsolutePosition(glm::vec3 terrainPos, float terrainWidth,
                                    float terrainHeight) {
  return glm::vec2(terrainPos.x - terrainWidth / 2 + radius +
                       relativePosition.x * (terrainWidth - 2 * radius),
                   terrainPos.z - terrainHeight / 2 + radius +
                       relativePosition.y * (terrainHeight - 2 * radius));
}

float Goal::getBottomHeight() { return goalModel.getBottomHeight(); }
