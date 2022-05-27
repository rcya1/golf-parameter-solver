#include "PerspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace opengl {
PerspectiveCamera::PerspectiveCamera(glm::vec3 &cameraPos, float yaw,
                                     float pitch, float fov, float aspectRatio)
    : cameraPos(cameraPos),
      yaw(yaw),
      pitch(pitch),
      fov(fov),
      aspectRatio(aspectRatio) {
  recalculateMatrices();
}

void PerspectiveCamera::recalculateMatrices() {
  cameraFront = glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                          sin(glm::radians(pitch)),
                          sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
  view = glm::lookAt(cameraPos, this->cameraPos + this->cameraFront,
                     glm::vec3(0.0, 1.0, 0.0));
  projection = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
}

void PerspectiveCamera::setPos(glm::vec3 &cameraPos) {
  this->cameraPos = cameraPos;
  recalculateMatrices();
}

void PerspectiveCamera::setRotation(float yaw, float pitch) {
  this->yaw = yaw;
  this->pitch = pitch;
  recalculateMatrices();
}

void PerspectiveCamera::setFOV(float fov) {
  this->fov = fov;
  recalculateMatrices();
}

void PerspectiveCamera::setAspectRatio(float aspectRatio) {
  this->aspectRatio = aspectRatio;
  recalculateMatrices();
}

glm::f32 *PerspectiveCamera::getViewMatrix() {
  return glm::value_ptr(view);
}

glm::f32 *PerspectiveCamera::getProjectionMatrix() {
  return glm::value_ptr(projection);
}
}  // namespace opengl
