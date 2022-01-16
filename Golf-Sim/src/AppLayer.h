#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>
#include <GLFW/glfw3.h>
#include <reactphysics3d/reactphysics3d.h>

#include <memory>

#include "ball/Ball.h"
#include "lights/Lights.h"
#include "terrain/Terrain.h"
#include "goal/Goal.h"
#include "util/opengl/PerspectiveCameraController.h"
#include "util/plot/TimeMetrics.h"
#include <terrain/TerrainRenderer.h>
#include <goal/GoalRenderer.h>
#include <util/opengl/DepthFrameBuffer.h>

class AppLayer : public GLCore::Layer {
 public:
  AppLayer(GLFWwindow* window);
  virtual ~AppLayer();

  virtual void OnAttach() override;
  virtual void OnDetach() override;
  virtual void OnEvent(GLCore::Event& event) override;
  virtual void update(GLCore::Timestep ts) override;
  void render();
  virtual void imGuiRender() override;

 private:
  GLFWwindow* window;
  bool isCursorControllingCamera;

  opengl::PerspectiveCameraController cameraController;
  lights::LightScene lightScene;

  opengl::DepthFrameBuffer lightDepthFrameBuffer0;
  opengl::Shader lightDepthShader;

  std::vector<Ball> balls;
  BallModel ballModel;
  BallRenderer ballRenderer;

  Terrain terrain;
  TerrainRenderer terrainRenderer;
  
  Goal goal;
  GoalRenderer goalRenderer;

  TimeMetrics timeMetrics;

  float physicsAccumulatedTime = 0;
  bool physicsRunning = true;
  bool justStartedPhysics = true;
  reactphysics3d::PhysicsCommon physicsCommon;
  reactphysics3d::PhysicsWorld* physicsWorld;
};
