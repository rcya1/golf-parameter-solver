#include "Ball.h"

#include <GLCoreUtils.h>

#include <iostream>

#include "BallRenderer.h"

Ball::Ball() : Ball(0.0f, 0.0f, 0.0f, 1.0f, glm::vec3(0.808f, 0.471f, 0.408f)) {}

Ball::Ball(float x, float y, float z, float r, glm::vec3 color)
    : position(x, y, z), radius(r), color(color) {}

void Ball::update(GLCore::Timestep ts, float interpolationFactor) {
  if (this->rigidBody == nullptr || interpolationFactor == -1) {
    return;
  }

  reactphysics3d::Transform currTransform = this->rigidBody->getTransform();

  reactphysics3d::Transform interpolatedTransform =
      reactphysics3d::Transform::interpolateTransforms(
          prevTransform, currTransform, interpolationFactor);

  prevTransform = currTransform;
  position =
      glm::vec3(currTransform.getPosition().x, currTransform.getPosition().y,
                currTransform.getPosition().z);
}

void Ball::render() {
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(position.x, position.y, position.z));
  model = glm::scale(model, glm::vec3(radius, radius, radius));
  BallRenderer::getInstance().add(BallRenderJob{model, color});
}

void Ball::imGuiRender(int index) {
  ImGui::PushID(index);
  ImGui::AlignTextToFramePadding();
  bool guiOpen = ImGui::TreeNode("Ball", "%s %u", "Ball", index);
  ImGui::NextColumn();
  if (guiOpen) {
    if (ImGui::DragFloat3("Position", glm::value_ptr(position), 1.0f, -10.0f,
                          10.0f)) {
      reactphysics3d::Vector3 position(position.x, position.y, position.z);
      reactphysics3d::Quaternion orientation =
          reactphysics3d::Quaternion::identity();
      reactphysics3d::Transform newTransform(position, orientation);

      rigidBody->setTransform(newTransform);
      rigidBody->setLinearVelocity(reactphysics3d::Vector3::zero());
      rigidBody->setAngularVelocity(reactphysics3d::Vector3::zero());
    }
    ImGui::ColorEdit3("Color", glm::value_ptr(color));
    ImGui::TreePop();
  }
  ImGui::PopID();
}

void Ball::addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                      reactphysics3d::PhysicsCommon& physicsCommon) {
  reactphysics3d::Vector3 position(position.x, position.y, position.z);
  reactphysics3d::Quaternion orientation =
      reactphysics3d::Quaternion::identity();
  reactphysics3d::Transform transform(position, orientation);

  this->prevTransform = transform;

  this->rigidBody = physicsWorld->createRigidBody(transform);
  this->rigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
  this->rigidBody->setMass(radius * radius * radius);

  reactphysics3d::SphereShape* sphereShape =
      physicsCommon.createSphereShape(radius);
  reactphysics3d::Transform shapeTransform =
      reactphysics3d::Transform::identity();

  this->collider = this->rigidBody->addCollider(sphereShape, shapeTransform);
}

void Ball::removePhysics(reactphysics3d::PhysicsWorld* physicsWorld) {
  physicsWorld->destroyRigidBody(this->rigidBody);
}
