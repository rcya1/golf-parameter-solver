#include "AppLayer.h"

#include <reactphysics3d/reactphysics3d.h>

#include "ball/BallRenderer.h"
#include "lights/Lights.h"
#include "terrain/PerlinNoise.h"
#include "util/PerspectiveCameraController.h"

using namespace GLCore;
using namespace GLCore::Utils;

AppLayer::AppLayer()
    : cameraController(glm::vec3(0, 5.0, 5.0), -90, 0, 45.0, 16.0 / 9.0, 3.0,
                       5.0, 0.1),
      ball(0.0, 10.0f, 0.0, 0.5),
      terrain(glm::vec3(0.0, -5.0, 0.0), 100, 100, 10.0f, 10.0f) {
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
  ball.addPhysics(physicsWorld, physicsCommon);
}

AppLayer::~AppLayer() {}

void AppLayer::OnAttach() {
  EnableGLDebugging();

  glEnable(GL_DEPTH_TEST);
}

void AppLayer::OnDetach() {
  BallRenderer::getInstance().freeRenderer();
  terrain.freeModel();

  terrain.removePhysics(physicsWorld);
  ball.removePhysics(physicsWorld);
  physicsCommon.destroyPhysicsWorld(physicsWorld);
}

void AppLayer::OnEvent(Event& event) {
  cameraController.OnEvent(event);

  EventDispatcher dispatcher(event);
}

void AppLayer::update(Timestep ts) {
  physicsAccumulatedTime += ts;
  float desiredTimeStep = 1.0 / 60.0f;
  while (physicsAccumulatedTime >= desiredTimeStep) {
    physicsAccumulatedTime -= desiredTimeStep;
    physicsWorld->update(desiredTimeStep);
  }

  cameraController.update(ts);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float interpolationFactor = physicsAccumulatedTime / desiredTimeStep;
  ball.update(ts, interpolationFactor);
  terrain.update(ts, interpolationFactor);

  ball.render();
  BallRenderer::getInstance().render(cameraController.getCamera(), lightScene);
  terrain.render(cameraController.getCamera(), lightScene);
}

void AppLayer::imGuiRender() {}
