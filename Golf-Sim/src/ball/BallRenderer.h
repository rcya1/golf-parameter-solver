#pragma once

#include <GLCoreUtils.h>
#include <ball/BallModel.h>

#include <glm/glm.hpp>
#include <queue>

#include "lights/Lights.h"
#include "util/opengl/PerspectiveCamera.h"
#include "util/opengl/Shader.h"

struct BallRenderJob {
  glm::mat4 model;
  glm::vec3 color;
};

class BallRenderer {
 public:
  BallRenderer();

  void render(BallModel& ballModel, opengl::PerspectiveCamera& camera,
              lights::LightScene& lightScene);
  void renderLightDepth(BallModel& ballModel, opengl::Shader& lightDepthShader,
                        lights::LightScene& lightScene, int dirLightIndex);
  void add(BallRenderJob job);
  void freeRenderer();
  void reloadShader() { shader.load(); }

 private:
  opengl::Shader shader;
  std::queue<BallRenderJob> queue;
};
