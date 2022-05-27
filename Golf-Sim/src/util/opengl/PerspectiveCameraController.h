#pragma once

#include <GLCore.h>

#include "PerspectiveCamera.h"

namespace opengl {
class PerspectiveCameraController {
 public:
  PerspectiveCameraController(glm::vec3& cameraPos, float yaw, float pitch,
                              float fov, float aspectRatio, float moveSpeed,
                              float zoomSpeed, float panSensitivity);

  void update(GLCore::Timestep ts);
  void OnEvent(GLCore::Event& e, bool isCursorControllingCamera);

  PerspectiveCamera& getCamera() { return camera; }
  const PerspectiveCamera& getCamera() const { return camera; }

 private:
  bool OnMouseScrolled(GLCore::MouseScrolledEvent& e);
  bool OnWindowResized(GLCore::WindowResizeEvent& e);
  bool OnMouseMoved(GLCore::MouseMovedEvent& e);

  PerspectiveCamera camera;
  float moveSpeed;
  float zoomSpeed;
  float panSensitivity;

  float lastMouseX;
  float lastMouseY;
  bool firstMouse = true;
};
}  // namespace opengl
