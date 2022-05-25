#pragma once

#include <queue>

#include <terrain/TerrainModel.h>
#include <util/opengl/Shader.h>
#include "lights/Lights.h"
#include "util/opengl/PerspectiveCamera.h"

struct TerrainRenderJob {
  TerrainModel& model;
  glm::vec3 position;
  glm::vec3 color;
  float goalLeft;
  float goalRight;
  float goalBottom;
  float goalTop;
};

class TerrainRenderer {
 public:
  TerrainRenderer();

  void render(opengl::PerspectiveCamera& camera,
              lights::LightScene& lightScene);
  void renderLightDepth(opengl::Shader& lightDepthShader,
                        lights::LightScene& lightScene, int dirLightIndex);
  void add(TerrainRenderJob job);
  void freeRenderer();
  void reloadShader() { shader.load(); }

 private:
  opengl::Shader shader;
  std::queue<TerrainRenderJob> queue;
};
