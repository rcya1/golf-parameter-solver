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
#include "util/opengl/RenderFrameBuffer.h"
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

  void initializeBallsSimultaneous();
  void initializeBallsStaggered();
  void addBall(float power, float yawOffset, float pitch);

 private:
  GLFWwindow* window;
  bool isCursorControllingCamera;
  opengl::RenderFrameBuffer renderFrameBuffer;

  opengl::PerspectiveCameraController cameraController;
  lights::LightScene lightScene;

  opengl::DepthFrameBuffer lightDepthFrameBuffer0;
  opengl::Shader lightDepthShader;
  
  opengl::Shader visualizeNormalsShader;
  opengl::Shader primitiveShader;

  bool showSidebar = true;
  bool showTimeMetrics = false;
  bool initSimultaneous = true;
  float dpiScale = 1.0;
  bool updateFont = false;

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

  const float PI = 3.14159265f;
  int paramsNumDivisions = 6;
  float minPower = 3.0;
  float maxPower = 15.0;
  float minPitch = 0.0;
  float maxPitch = PI / 3;
  float minYaw = -PI / 4;
  float maxYaw = PI / 4;

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
