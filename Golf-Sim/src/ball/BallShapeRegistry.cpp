#include "BallShapeRegistry.h"

reactphysics3d::SphereShape* BallShapeRegistry::getShape(
    float radius, reactphysics3d::PhysicsCommon& physicsCommon) {
  if (shapes.find(radius) != shapes.end()) {
    freq[radius] += 1;
    return shapes[radius];
  }

  shapes.insert(
      std::make_pair(radius, physicsCommon.createSphereShape(radius)));
  freq.insert(std::make_pair(radius, 1));

  return shapes[radius];
}

void BallShapeRegistry::removeUsage(
    float radius, reactphysics3d::PhysicsCommon& physicsCommon) {
  freq[radius] -= 1;
  if (freq[radius] = 0) {
    physicsCommon.destroySphereShape(shapes[radius]);
    shapes.erase(radius);
    freq.erase(radius);
  }
}
