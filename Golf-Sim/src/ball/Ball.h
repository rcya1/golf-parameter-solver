#pragma once

#include <GLCore.h>
#include <glm/glm.hpp>
#include <reactphysics3d/reactphysics3d.h>

class Terrain;
class Goal;
class BallRenderer;

enum class BallState { ACTIVE, OUT_OF_BOUNDS, STATIONARY, GOAL };

class Ball {
 public:
  Ball();
  Ball(float x, float y, float z, float r, glm::vec3 color);

  void update(GLCore::Timestep ts, Terrain& terrain, Goal& goal, reactphysics3d::PhysicsWorld* physicsWorld,
    reactphysics3d::PhysicsCommon& physicsCommon, float interpolationFactor = -1);
  void render(BallRenderer& renderer);
  void imGuiRender(int index, reactphysics3d::PhysicsWorld* physicsWorld,
    reactphysics3d::PhysicsCommon& physicsCommon);

  void addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                  reactphysics3d::PhysicsCommon& physicsCommon);
  void removePhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                     reactphysics3d::PhysicsCommon& physicsCommon);

  void setVelocity(glm::vec3 velocity);
  void setState(BallState state) { this->state = state; }
  glm::vec3 getPosition() { return position; }
  bool hasPhysics() { return rigidBody != nullptr; }
  bool isOutOfBounds(Terrain& terrain);

 private:
  const float BOUNCINESS = 0.2;
  const float FRICTION = 0.6;
  const float MATERIAL_DENSITY = 10.0;

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
