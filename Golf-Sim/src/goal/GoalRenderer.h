#pragma once

#include <goal/GoalModel.h>

#include <util/opengl/Shader.h>

#include <queue>

namespace opengl {
class PerspectiveCamera;
}

namespace lights {
class LightScene;
}

struct GoalRenderJob {
  GoalModel& model;
  glm::vec3 color;
};

class GoalRenderer {
 public:
  GoalRenderer();

  void render(opengl::PerspectiveCamera& camera, lights::LightScene& lightScene,
              opengl::Shader* shader = nullptr);
  void renderLightDepth(opengl::Shader& lightDepthShader,
                        lights::LightScene& lightScene, int dirLightIndex);
  void add(GoalRenderJob job);
  void freeRenderer();
  void reloadShader() { shader.load(); }

 private:
  opengl::Shader shader;
  std::queue<GoalRenderJob> queue;
};
