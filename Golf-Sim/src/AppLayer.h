#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>
#include <reactphysics3d/reactphysics3d.h>

#include <memory>

#include "ball/Ball.h"
#include "lights/Lights.h"
#include "terrain/Terrain.h"
#include "util/PerspectiveCameraController.h"

class AppLayer : public GLCore::Layer {
 public:
  AppLayer();
  virtual ~AppLayer();

  virtual void OnAttach() override;
  virtual void OnDetach() override;
  virtual void OnEvent(GLCore::Event& event) override;
  virtual void update(GLCore::Timestep ts) override;
  virtual void imGuiRender() override;

 private:
  opengl::PerspectiveCameraController cameraController;
  lights::LightScene lightScene;
  Ball ball;
  Terrain terrain;

  float physicsAccumulatedTime = 0;
  reactphysics3d::PhysicsCommon physicsCommon;
  reactphysics3d::PhysicsWorld* physicsWorld;
};
