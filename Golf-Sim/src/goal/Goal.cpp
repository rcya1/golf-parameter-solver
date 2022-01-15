#include "Goal.h"

Goal::Goal(float x, float z, float r) : position(x, z), radius(r) {}

void Goal::generateModel(Terrain& terrain) {
  if (vertices.size() > 0) {
    freeModel();
  }

  for(int i = 0; i < SECTOR_COUNT; i++) {
    float sectorAngle = SECTOR_STEP * i;
    float x = radius * cosf(sectorAngle);
    float z = radius * sinf(sectorAngle);
  }
}

void Goal::freeModel() {
  vertices.clear();
}

void Goal::addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                      reactphysics3d::PhysicsCommon& physicsCommon) {}

void Goal::removePhysics(reactphysics3d::PhysicsWorld* physicsWorld) {}
