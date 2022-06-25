#pragma once

#include "ball/Ball.h"
#include "ball/BallModel.h"
#include "ball/BallRenderer.h"
#include "terrain/Terrain.h"
#include "terrain/TerrainRenderer.h"
#include "goal/Goal.h"
#include "goal/GoalRenderer.h"
#include "lights/Lights.h"

#include "util/opengl/PerspectiveCameraController.h"
#include "util/plot/TimeMetrics.h"
#include "util/opengl/DepthFrameBuffer.h"

#include <GLCore.h>
#include <GLCoreUtils.h>
#include <GLFW/glfw3.h>
#include <reactphysics3d/reactphysics3d.h>

#include <memory>

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
  
  opengl::Shader visualizeNormalsShader;
  opengl::Shader primitiveShader;

  std::vector<Ball> balls;
  std::queue<Ball> ballsAdd;
  BallModel ballModel;
  BallRenderer ballRenderer;

  glm::vec3 addBallPosition;
  float addBallRadius;
  glm::vec3 addBallColor;
  bool addBallHasPhysics;

  glm::vec2 startPosition;
  float startPositionHighlightRadius;
  glm::vec3 startPositionHighlightColor;

  Terrain terrain;
  TerrainRenderer terrainRenderer;
  
  Goal goal;
  GoalRenderer goalRenderer;

  TimeMetrics timeMetrics;

  float physicsAccumulatedTime = 0;
  bool physicsRunning = false;
  bool justStartedPhysics = true;
  reactphysics3d::PhysicsCommon physicsCommon;
  reactphysics3d::PhysicsWorld* physicsWorld;

  bool renderShadows = false;
  bool renderNormals = false;
  bool renderPhysicsDebugging = false;
};
