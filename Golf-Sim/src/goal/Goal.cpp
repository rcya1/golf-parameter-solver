#include "Goal.h"

#include <goal/GoalRenderer.h>

Goal::Goal(float x, float z, float r)
    : position(x, z), radius(r), color(0.1f, 0.35f, 0.1f) {}

void Goal::generateModel(Terrain& terrain) {
  freeModel();

  glm::vec2 terrain2DCoords =
      glm::vec2(terrain.getPosition().x - terrain.getWidth() / 2,
                terrain.getPosition().z - terrain.getHeight() / 2);
  glm::vec2 relativeCoords = position - terrain2DCoords;

  if (relativeCoords.x + radius > terrain.getWidth() ||
      relativeCoords.y + radius > terrain.getHeight() ||
      relativeCoords.x < radius || relativeCoords.y < radius) {
    assert(false);
  }

  goalModel.generateModel(terrain, relativeCoords, radius);
}

void Goal::freeModel() { goalModel.freeModel(); }

void Goal::render(GoalRenderer& renderer) {
  renderer.add(GoalRenderJob{goalModel, color});
}

void Goal::addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                      reactphysics3d::PhysicsCommon& physicsCommon) {}

void Goal::removePhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                         reactphysics3d::PhysicsCommon& physicsCommon) {}
