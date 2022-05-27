#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace opengl {
class PerspectiveCamera {
 public:
  PerspectiveCamera(glm::vec3& cameraPos, float yaw, float pitch, float fov,
                    float aspectRatio);

  void recalculateMatrices();
  glm::f32* getViewMatrix();
  glm::f32* getProjectionMatrix();

  void setPos(glm::vec3& cameraPos);
  void setRotation(float yaw, float pitch);
  void setFOV(float fov);
  void setAspectRatio(float aspectRatio);

  glm::vec3 getPos() { return cameraPos; }
  glm::vec3 getFront() { return cameraFront; }

  float getYaw() { return yaw; }
  float getPitch() { return pitch; }
  float getFOV() { return fov; }
  float getAspectRatio() { return aspectRatio; }

 private:
  glm::mat4 view;
  glm::mat4 projection;

  glm::vec3 cameraPos;
  glm::vec3 cameraFront;
  float yaw;
  float pitch;
  float fov;
  float aspectRatio;
};
}  // namespace opengl
