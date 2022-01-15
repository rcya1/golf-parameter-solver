#pragma once

#include <queue>

#include <goal/GoalModel.h>
#include <util/opengl/Shader.h>
#include "lights/Lights.h"
#include "util/opengl/PerspectiveCamera.h"

struct GoalRenderJob {
  GoalModel& model;
  glm::vec3 position;
  glm::vec3 color;
};

class GoalRenderer {
 public:
  GoalRenderer();

  void render(opengl::PerspectiveCamera& camera,
              lights::LightScene& lightScene);
  void add(GoalRenderJob job);
  void freeRenderer();

 private:
  opengl::Shader shader;
  std::queue<GoalRenderJob> queue;
};
