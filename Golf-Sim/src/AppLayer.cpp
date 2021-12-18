#include "AppLayer.h"

#include <GLFW/glfw3.h>
#include <reactphysics3d/reactphysics3d.h>

#include "GLCore/Core/Input.h"
#include "GLCore/Core/KeyCodes.h"
#include "ball/BallRenderer.h"
#include "lights/Lights.h"
#include "terrain/PerlinNoise.h"
#include "util/opengl/PerspectiveCameraController.h"
#include "implot.h"

using namespace GLCore;
using namespace GLCore::Utils;

AppLayer::AppLayer(GLFWwindow* window)
    : window(window),
      cameraController(glm::vec3(0, 5.0, 5.0), -90, 0, 45.0, 16.0 / 9.0, 3.0,
                       5.0, 0.1),
      balls{Ball(-1.0, 10.0f, 0.0, 0.5, glm::vec3(0.808f, 0.471f, 0.408f)),
            Ball(1.0, 10.0f, 0.0, 0.5, glm::vec3(0.408f, 0.471f, 0.808f))},
      terrain(glm::vec3(0.0, -5.0, 0.0), 10, 10, 10.0f, 10.0f) {
  lightScene = lights::LightScene{
      std::vector<lights::PointLight>{
          lights::createBasicPointLight(glm::vec3(25.0f, 25.0f, 25.0f)),
          lights::createBasicPointLight(glm::vec3(25.0f, 25.0f, -25.0f)),
          lights::createBasicPointLight(glm::vec3(25.0f, -25.0f, 25.0f)),
          lights::createBasicPointLight(glm::vec3(25.0f, -25.0f, -25.0f)),
          lights::createBasicPointLight(glm::vec3(-25.0f, 25.0f, 25.0f)),
          lights::createBasicPointLight(glm::vec3(-25.0f, 25.0f, -25.0f)),
          lights::createBasicPointLight(glm::vec3(-25.0f, -25.0f, 25.0f)),
          lights::createBasicPointLight(glm::vec3(-25.0f, -25.0f, -25.0f)),
      },
      std::vector<lights::DirLight>{
          lights::createBasicDirLight(glm::vec3(0.0f, -1.0f, 0.0f))}};

  noise::initNoise();
  terrain.generateModel(5.0f, 5.0f);

  physicsWorld = physicsCommon.createPhysicsWorld();

  terrain.addPhysics(physicsWorld, physicsCommon);
  for (Ball& ball : balls) {
    ball.addPhysics(physicsWorld, physicsCommon);
  }
}

AppLayer::~AppLayer() {}

void AppLayer::OnAttach() {
  EnableGLDebugging();

  glEnable(GL_DEPTH_TEST);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  isCursorControllingCamera = true;
}

void AppLayer::OnDetach() {
  BallRenderer::getInstance().freeRenderer();
  terrain.freeModel();

  terrain.removePhysics(physicsWorld);
  for (Ball& ball : balls) {
    ball.removePhysics(physicsWorld);
  }
  physicsCommon.destroyPhysicsWorld(physicsWorld);
}

void AppLayer::OnEvent(Event& event) {
  EventDispatcher dispatcher(event);
  dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e) {
    if (e.GetKeyCode() == HZ_KEY_ESCAPE) {
      if (isCursorControllingCamera) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        isCursorControllingCamera = false;
      } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        isCursorControllingCamera = true;
      }
    }

    return false;
  });

  cameraController.OnEvent(event, isCursorControllingCamera);
}

void AppLayer::update(Timestep ts) {
  if (!justStartedPhysics && physicsRunning) {
    physicsAccumulatedTime += ts;
  }
  if (physicsRunning) {
    justStartedPhysics = false;
  }
  float desiredPhysicsTimeStep = 1.0 / 60.0f;
  while (physicsAccumulatedTime >= desiredPhysicsTimeStep && physicsRunning) {
    physicsAccumulatedTime -= desiredPhysicsTimeStep;
    physicsWorld->update(desiredPhysicsTimeStep);
  }

  cameraController.update(ts);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float interpolationFactor =
      physicsRunning ? physicsAccumulatedTime / desiredPhysicsTimeStep : 0;
  for (Ball& ball : balls) {
    ball.update(ts, interpolationFactor);
  }
  terrain.update(ts, interpolationFactor);

  for (Ball& ball : balls) {
    ball.render();
  }
  BallRenderer::getInstance().render(cameraController.getCamera(), lightScene);
  terrain.render(cameraController.getCamera(), lightScene);

  timeMetrics.update(ts);
}

void AppLayer::imGuiRender() {
  ImGui::Begin("Physics");
  ImGui::SetWindowFontScale(2.0);
  if (ImGui::Button("Start / Stop")) {
    physicsRunning = !physicsRunning;
    if (physicsRunning) {
      justStartedPhysics = true;
    }
  }
  ImGui::End();

  ImGui::Begin("Balls");
  ImGui::SetWindowFontScale(2.0);
  for (int i = 0; i < balls.size(); i++) {
    balls[i].imGuiRender(i);
  }
  ImGui::End();

  terrain.imGuiRender();
  timeMetrics.imGuiRender();
}
