#include "AppLayer.h"

#include <GLFW/glfw3.h>
#include <reactphysics3d/reactphysics3d.h>

#include "GLCore/Core/Input.h"
#include "GLCore/Core/KeyCodes.h"
#include "ball/BallRenderer.h"
#include "implot.h"
#include "lights/Lights.h"
#include "terrain/PerlinNoise.h"
#include "util/opengl/PerspectiveCameraController.h"

using namespace GLCore;
using namespace GLCore::Utils;

AppLayer::AppLayer(GLFWwindow* window)
    : window(window),
      cameraController(glm::vec3(0, 5.0, 5.0), -90, 0, 45.0, 2.0 / 1.0, 3.0,
                       5.0, 0.1),
      balls{Ball(-1.0, 5.0f, 0.0, 0.25, glm::vec3(0.808f, 0.471f, 0.408f)),
            Ball(1.0, 5.0f, 0.0, 0.25, glm::vec3(0.408f, 0.471f, 0.808f))},
      goal(0, 0, 0.5),
      terrain(glm::vec3(0.0, -5.0, 0.0), 25, 25, 10.0f, 10.0f, 5.0f, 5.0f) {
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

  goal.generateModel(terrain);

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
  ballModel.freeModel();
  ballRenderer.freeRenderer();
  terrain.freeModel();
  goal.freeModel();

  terrain.removePhysics(physicsWorld, physicsCommon);
  for (Ball& ball : balls) {
    ball.removePhysics(physicsWorld, physicsCommon);
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
  dispatcher.Dispatch<GLCore::WindowResizeEvent>([&](WindowResizeEvent& e) {
    glViewport(0, 0, e.GetWidth(), e.GetHeight());
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
    ball.update(ts, goal, interpolationFactor);
    if (ball.hasPhysics() &&
        ball.getPosition().y <
            terrain.getPosition().y + terrain.getMinHeight()) {
      ball.setState(BallState::OUT_OF_BOUNDS);
      ball.removePhysics(physicsWorld, physicsCommon);
    }
  }
  terrain.update(ts, interpolationFactor);

  for (Ball& ball : balls) {
    ball.render(ballRenderer);
  }
  ballRenderer.render(ballModel, cameraController.getCamera(), lightScene);

  terrain.render(terrainRenderer, goal.getPosition(), goal.getRadius());
  terrainRenderer.render(cameraController.getCamera(), lightScene);

  timeMetrics.update(ts);
}

void AppLayer::imGuiRender() {
  ImGui::Begin("Physics");
  if (ImGui::Button("Start / Stop")) {
    physicsRunning = !physicsRunning;
    if (physicsRunning) {
      justStartedPhysics = true;
    }
  }
  ImGui::End();

  ImGui::Begin("Balls");
  for (int i = 0; i < balls.size(); i++) {
    balls[i].imGuiRender(i);
  }
  ImGui::End();

  terrain.imGuiRender(physicsWorld, physicsCommon);
  timeMetrics.imGuiRender();
}
