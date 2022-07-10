#include "Ball.h"

#include <imgui.h>

#include <iostream>

#include "BallRenderer.h"
#include "BallShapeRegistry.h"
#include "goal/Goal.h"
#include "terrain/Terrain.h"
#include "util/CollisionCategory.h"

Ball::Ball()
    : Ball(0.0f, 0.0f, 0.0f, 1.0f, glm::vec3(0.808f, 0.471f, 0.408f)) {}

Ball::Ball(float x, float y, float z, float r, glm::vec3 color)
    : position(x, y, z),
      radius(r),
      color(color),
      state(BallState::ACTIVE),
      nearGoal(false),
      rigidBody(nullptr) {}

void Ball::update(GLCore::Timestep ts, Terrain& terrain, Goal& goal,
                  reactphysics3d::PhysicsWorld* physicsWorld,
                  reactphysics3d::PhysicsCommon& physicsCommon,
                  BallShapeRegistry& ballShapeRegistry,
                  float interpolationFactor) {
  if (!isOutOfBounds(terrain) && state == BallState::OUT_OF_BOUNDS) {
    if (!hasPhysics()) {
      addPhysics(physicsWorld, physicsCommon, ballShapeRegistry);
    }
    state = BallState::ACTIVE;
  }

  if (!hasPhysics() || interpolationFactor == -1) {
    return;
  }

  if (isOutOfBounds(terrain) && hasPhysics()) {
    state = BallState::OUT_OF_BOUNDS;
    removePhysics(physicsWorld, physicsCommon, ballShapeRegistry);
    return;
  }

  if (state == BallState::STATIONARY || state == BallState::ACTIVE) {
    if (this->rigidBody->getLinearVelocity().lengthSquare() < 0.05) {
      state = BallState::STATIONARY;
    } else {
      state = BallState::ACTIVE;
    }
  }

  reactphysics3d::Transform currTransform = this->rigidBody->getTransform();

  reactphysics3d::Transform interpolatedTransform =
      reactphysics3d::Transform::interpolateTransforms(
          prevTransform, currTransform, interpolationFactor);

  prevTransform = currTransform;
  position = glm::vec3(interpolatedTransform.getPosition().x,
                       interpolatedTransform.getPosition().y,
                       interpolatedTransform.getPosition().z);

  float goalDx =
      currTransform.getPosition().x - goal.getAbsolutePosition(terrain).x;
  float goalDz =
      currTransform.getPosition().z - goal.getAbsolutePosition(terrain).y;
  float switchRadius = goal.getRadius() + radius;
  bool newNearGoal =
      goalDx * goalDx + goalDz * goalDz < switchRadius * switchRadius;
  // have to remove and add the collider because otherwise sometimes it doesn't
  // update properly
  if (!nearGoal && newNearGoal) {
    this->rigidBody->removeCollider(this->collider);
    this->collider = this->rigidBody->addCollider(
        sphereShape, reactphysics3d::Transform::identity());
    this->collider->setCollisionCategoryBits(CollisionCategory::BALL);
    this->collider->setCollideWithMaskBits(CollisionCategory::GOAL);
    this->collider->getMaterial().setBounciness(BOUNCINESS);
    this->collider->getMaterial().setFrictionCoefficient(FRICTION);
    this->collider->getMaterial().setMassDensity(MATERIAL_DENSITY);
    nearGoal = true;
  } else if (nearGoal && !newNearGoal) {
    this->rigidBody->removeCollider(this->collider);
    this->collider = this->rigidBody->addCollider(
        sphereShape, reactphysics3d::Transform::identity());
    this->collider->setCollisionCategoryBits(CollisionCategory::BALL);
    this->collider->setCollideWithMaskBits(CollisionCategory::TERRAIN);
    this->collider->getMaterial().setBounciness(BOUNCINESS);
    this->collider->getMaterial().setFrictionCoefficient(FRICTION);
    this->collider->getMaterial().setMassDensity(MATERIAL_DENSITY);
    nearGoal = false;
  }

  // calculate if we are in the goal or not based on our current height and
  // proximity to the goal
  if (state == BallState::STATIONARY &&
      abs(position.y - radius - goal.getBottomHeight()) < 0.2 && newNearGoal) {
    state = BallState::GOAL;
  }
}

void Ball::render(BallRenderer& renderer) {
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(position.x, position.y, position.z));
  model = glm::scale(model, glm::vec3(radius, radius, radius));
  renderer.add(BallRenderJob{model, color});
}

void Ball::imGuiRender(int index, reactphysics3d::PhysicsWorld* physicsWorld,
                       reactphysics3d::PhysicsCommon& physicsCommon,
                       BallShapeRegistry& ballShapeRegistry) {
  ImGui::PushID(index);
  ImGui::AlignTextToFramePadding();

  char* stateString = nullptr;
  switch (state) {
    case BallState::ACTIVE:
      stateString = "Active";
      break;
    case BallState::GOAL:
      stateString = "Goal";
      break;
    case BallState::OUT_OF_BOUNDS:
      stateString = "Out of Bounds";
      break;
    case BallState::STATIONARY:
      stateString = "Stationary";
      break;
  }

  bool guiOpen = ImGui::TreeNode("Ball", "Ball %u [%s]", index, stateString);
  ImGui::NextColumn();
  if (guiOpen) {
    if (ImGui::DragFloat3("Position", glm::value_ptr(position), 1.0f, -10.0f,
                          10.0f)) {
      if (hasPhysics()) {
        reactphysics3d::Vector3 position(position.x, position.y, position.z);
        reactphysics3d::Quaternion orientation =
            reactphysics3d::Quaternion::identity();
        reactphysics3d::Transform newTransform(position, orientation);

        rigidBody->setTransform(newTransform);
        rigidBody->setLinearVelocity(reactphysics3d::Vector3::zero());
        rigidBody->setAngularVelocity(reactphysics3d::Vector3::zero());

        prevTransform = newTransform;
      }
    }
    if (ImGui::DragFloat("Radius", &radius, 0.01f, 0.01f, 2.0f)) {
      if (hasPhysics()) {
        removePhysics(physicsWorld, physicsCommon, ballShapeRegistry);
        addPhysics(physicsWorld, physicsCommon, ballShapeRegistry);
      }
    }
    ImGui::ColorEdit3("Color", glm::value_ptr(color));
    if (nearGoal) {
      ImGui::Text("Near Goal");
    }

    if (!hasPhysics()) {
      bool addPhysicsDisabled = state == BallState::OUT_OF_BOUNDS;
      if (addPhysicsDisabled) {
        ImGui::BeginDisabled();
      }

      if (ImGui::Button("Add Physics")) {
        addPhysics(physicsWorld, physicsCommon, ballShapeRegistry);
      }
      if (addPhysicsDisabled) {
        ImGui::EndDisabled();
      }
    } else {
      if (ImGui::Button("Remove Physics")) {
        removePhysics(physicsWorld, physicsCommon, ballShapeRegistry);
      }
    }

    if (hasPhysics() && this->collider != nullptr)
      ImGui::Text(
          std::to_string(this->collider->getCollideWithMaskBits()).c_str());

    ImGui::TreePop();
  }
  ImGui::PopID();
}

void Ball::addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                      reactphysics3d::PhysicsCommon& physicsCommon,
                      BallShapeRegistry& ballShapeRegistry) {
  reactphysics3d::Vector3 position(position.x, position.y, position.z);
  reactphysics3d::Quaternion orientation =
      reactphysics3d::Quaternion::identity();
  reactphysics3d::Transform transform(position, orientation);

  this->prevTransform = transform;

  this->rigidBody = physicsWorld->createRigidBody(transform);
  this->rigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
  this->rigidBody->setMass(radius * radius * radius);
  this->rigidBody->setLinearDamping(0.3);

  this->sphereShape = ballShapeRegistry.getShape(radius, physicsCommon);

  this->collider = this->rigidBody->addCollider(
      sphereShape, reactphysics3d::Transform::identity());
  this->collider->setCollisionCategoryBits(CollisionCategory::BALL);
  this->collider->setCollideWithMaskBits(CollisionCategory::TERRAIN);
  this->collider->getMaterial().setBounciness(BOUNCINESS);
  this->collider->getMaterial().setFrictionCoefficient(FRICTION);
  this->collider->getMaterial().setMassDensity(MATERIAL_DENSITY);
}

void Ball::removePhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                         reactphysics3d::PhysicsCommon& physicsCommon,
                         BallShapeRegistry& ballShapeRegistry) {
  if (!hasPhysics()) return;

  physicsWorld->destroyRigidBody(this->rigidBody);
  ballShapeRegistry.removeUsage(radius, physicsCommon);

  this->rigidBody = nullptr;
  this->collider = nullptr;
  this->sphereShape = nullptr;
}

void Ball::setVelocity(glm::vec3 velocity) {
  this->rigidBody->setLinearVelocity(
      reactphysics3d::Vector3(velocity.x, velocity.y, velocity.z));
}

bool Ball::isOutOfBounds(Terrain& terrain) {
  return position.y < terrain.getPosition().y + terrain.getMinHeight();
}

float Ball::getDistFromGoal(Goal& goal, Terrain& terrain) {
  glm::vec2 goalPos = goal.getAbsolutePosition(terrain);
  glm::vec2 ball2DPos = glm::vec2(position.x, position.z);

  return glm::length(goalPos - ball2DPos);
}
