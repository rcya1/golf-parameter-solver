#pragma once

#include <reactphysics3d/reactphysics3d.h>

#include <map>

class BallShapeRegistry {
 public:
  reactphysics3d::SphereShape *getShape(
      float radius, reactphysics3d::PhysicsCommon &physicsCommon);

  void removeUsage(float radius, reactphysics3d::PhysicsCommon &physicsCommon);

 private:
  std::map<float, reactphysics3d::SphereShape *> shapes;
  std::map<float, int> freq;
};
