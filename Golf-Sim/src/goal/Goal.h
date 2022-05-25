#pragma once

#include <GLCore.h>
#include <reactphysics3d/reactphysics3d.h>
#include "terrain/Terrain.h"
#include <goal/GoalModel.h>
#include <goal/GoalRenderer.h>

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
  float getRadius() { return radius; }

 private:
  glm::vec2 relativePosition;
  float radius;
  glm::vec3 color;

  GoalModel goalModel;
};
