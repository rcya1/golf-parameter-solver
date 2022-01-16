#pragma once

#include <goal/GoalModel.h>
#include <util/opengl/Shader.h>

#include <queue>

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
  void renderLightDepth(opengl::Shader& lightDepthShader,
                        lights::LightScene& lightScene, int dirLightIndex);
  void add(GoalRenderJob job);
  void freeRenderer();

 private:
  opengl::Shader shader;
  std::queue<GoalRenderJob> queue;
};
