#pragma once

#include "goal/GoalModel.h"

#include <reactphysics3d/reactphysics3d.h>
#include <glm/glm.hpp>

class Terrain;
class GoalRenderer;

class Goal {
 public:
  const float PI = 3.14159265f;
  const int SECTOR_COUNT = 18;
  const float SECTOR_STEP = 2 * PI / SECTOR_COUNT;

  Goal(float x, float z, float r);

  void generateModel(Terrain& terrain);
  void freeModel();

  void render(GoalRenderer& renderer);
  void imGuiRender(reactphysics3d::PhysicsWorld* physicsWorld,
    reactphysics3d::PhysicsCommon& physicsCommon, Terrain& terrain);

  void addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                  reactphysics3d::PhysicsCommon& physicsCommon);
  void removePhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                     reactphysics3d::PhysicsCommon& physicsCommon);

  glm::vec2 getRelativePosition() { return relativePosition; }
  glm::vec2 getAbsolutePosition(Terrain& terrain);
  glm::vec2 getAbsolutePosition(glm::vec3 terrainPos, float terrainWidth,
                                float terrainHeight);
  float getRadius() { return radius; }
  float getBottomHeight();

 private:
  glm::vec2 relativePosition;
  float radius;
  glm::vec3 color;

  GoalModel goalModel;

  reactphysics3d::RigidBody* rigidBody;
  reactphysics3d::Collider* collider;
  reactphysics3d::ConcaveMeshShape* shape;
  reactphysics3d::TriangleMesh* triangleMesh;
  reactphysics3d::TriangleVertexArray* terrainVA;
  reactphysics3d::TriangleVertexArray* wallsVA;
  reactphysics3d::TriangleVertexArray* bottomVA;
  reactphysics3d::Transform prevTransform;
};
