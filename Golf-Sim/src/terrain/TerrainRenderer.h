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
  glm::vec2 goalPosition;
  float goalRadius;
};

class TerrainRenderer {
 public:
  TerrainRenderer();

  void render(opengl::PerspectiveCamera& camera, lights::LightScene& lightScene);
  void add(TerrainRenderJob job);
  void freeRenderer();

 private:
  opengl::Shader shader;
  std::queue<TerrainRenderJob> queue;
};
