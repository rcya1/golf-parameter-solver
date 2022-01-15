#pragma once

#include <GLCore.h>
#include <reactphysics3d/reactphysics3d.h>
#include "goal/Goal.h"
#include "ball/BallRenderer.h"

enum class BallState { ACTIVE, OUT_OF_BOUNDS, STATIONARY, GOAL };

class Ball {
 public:
  Ball();
  Ball(float x, float y, float z, float r, glm::vec3 color);

  void update(GLCore::Timestep ts, Goal& goal, float interpolationFactor = -1);
  void render(BallRenderer& renderer);
  void imGuiRender(int index);

  void addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                  reactphysics3d::PhysicsCommon& physicsCommon);
  void removePhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                     reactphysics3d::PhysicsCommon& physicsCommon);

  void setVelocity(glm::vec3 velocity);
  void setState(BallState state) { this->state = state; }
  glm::vec3 getPosition() { return position; }
  bool hasPhysics() { return rigidBody != nullptr; }

 private:
  glm::vec3 position;
  float radius;
  glm::vec3 color;
  BallState state;
  bool nearGoal;

  reactphysics3d::RigidBody* rigidBody;
  reactphysics3d::Collider* collider;
  reactphysics3d::SphereShape* sphereShape;
  reactphysics3d::Transform prevTransform;
};
