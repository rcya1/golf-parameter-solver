#include "Goal.h"
#include <goal/GoalRenderer.h>

Goal::Goal(float x, float z, float r) : position(x, z), radius(r) {}

void Goal::generateModel(Terrain& terrain) {
  freeModel();

  goalModel.generateModel();

  for(int i = 0; i < SECTOR_COUNT; i++) {
    float sectorAngle = SECTOR_STEP * i;
    float x = radius * cosf(sectorAngle);
    float z = radius * sinf(sectorAngle);
  }
}

void Goal::freeModel() { goalModel.freeModel(); }

void Goal::render(GoalRenderer& renderer) {
  //renderer.add(GoalRenderJob{goalModel});
}

void Goal::addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                      reactphysics3d::PhysicsCommon& physicsCommon) {}

void Goal::removePhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                         reactphysics3d::PhysicsCommon& physicsCommon) {}
