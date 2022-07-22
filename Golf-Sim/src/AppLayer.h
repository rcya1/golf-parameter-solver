#pragma once

#include "ball/Ball.h"
#include "ball/BallModel.h"
#include "ball/BallRenderer.h"
#include "ball/BallShapeRegistry.h"
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

 private:
  GLFWwindow* window;
  bool isCursorControllingCamera = false;
  opengl::RenderFrameBuffer renderFrameBuffer;

  opengl::PerspectiveCameraController cameraController;
  lights::LightScene lightScene;

  opengl::DepthFrameBuffer lightDepthFrameBuffer0;
  opengl::Shader lightDepthShader;
  
  opengl::Shader visualizeNormalsShader;
  opengl::Shader primitiveShader;

  bool showHelpPopup = true;
  bool showSidebar = true;
  
  bool showTerrainSettings = false;
  bool showGoalSettings = false;
  bool showBallSettings = false;
  bool showDebugWindows = false;

  bool showTimeMetrics = false;
  bool initSimultaneous = false;
  float dpiScale = 1.0;
  bool updateFont = false;

  std::vector<Ball> balls;
  std::queue<Ball> ballsAdd;
  BallModel ballModel;
  BallRenderer ballRenderer;
  BallShapeRegistry ballShapeRegistry;

  glm::vec3 addBallPosition;
  float addBallRadius;
  glm::vec3 addBallColor;
  bool addBallHasPhysics;

  glm::vec2 startPosition;
  float startPositionHighlightRadius;
  glm::vec3 startPositionHighlightColor;

  const float PI = 3.14159265f;
  int paramsNumDivisions = 10;
  float minPower = 15.0;
  float maxPower = 30.0;
  float minPitch = 30.0;
  float maxPitch = 60.0f;
  float minYaw = -15.0f;
  float maxYaw = 15.0f;
  int staggeredBatchSize = 150;
  std::string outputFilePath = "";
  bool exportReady = false;
  std::vector<glm::vec3> staggeredBalls;

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

  void initializeBalls(bool staggered);
  void addBall(float power, float yawOffset, float pitch, bool staggered);
  void addBall(glm::vec3 velocity);
  void writeOutputFile(std::ofstream &fout);
};
