#pragma once

#include <GLCore.h>
#include <reactphysics3d/reactphysics3d.h>

#include <memory>

class Ball {
 public:
  Ball();
  Ball(float x, float y, float z, float r);

  void update(GLCore::Timestep ts, float interpolationFactor = -1);
  void render();
  void imGuiRender();

  void addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                  reactphysics3d::PhysicsCommon& physicsCommon);
  void removePhysics(reactphysics3d::PhysicsWorld* physicsWorld);

 private:
  glm::vec3 position;
  float radius;

  reactphysics3d::RigidBody* rigidBody;
  reactphysics3d::Collider* collider;
  reactphysics3d::Transform prevTransform;
};
