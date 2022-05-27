#pragma once

#include "util/opengl/Shader.h"

#include <glm/glm.hpp>

#include <queue>

class BallModel;

namespace opengl {
class PerspectiveCamera;
}

namespace lights {
class LightScene;
}

struct BallRenderJob {
  glm::mat4 model;
  glm::vec3 color;
};

class BallRenderer {
 public:
  BallRenderer();

  void render(BallModel& ballModel, opengl::PerspectiveCamera& camera,
              lights::LightScene& lightScene, opengl::Shader* shader = nullptr);
  void renderLightDepth(BallModel& ballModel, opengl::Shader& lightDepthShader,
                        lights::LightScene& lightScene, int dirLightIndex);
  void add(BallRenderJob job);
  void freeRenderer();
  void reloadShader() { shader.load(); }

 private:
  opengl::Shader shader;
  std::queue<BallRenderJob> queue;
};
