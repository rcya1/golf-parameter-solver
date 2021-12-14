#pragma once

#include <GLCoreUtils.h>

#include <glm/glm.hpp>
#include <queue>

#include "lights/Lights.h"
#include "util/PerspectiveCamera.h"
#include "util/Shader.h"

class BallRenderer {
 public:
  static BallRenderer& getInstance() {
    static BallRenderer instance;
    return instance;
  }

  void render(opengl::PerspectiveCamera& camera,
              lights::LightScene& lightScene);
  void add(glm::mat4 model);
  void freeRenderer();

 private:
  opengl::Shader shader;
  std::queue<glm::mat4> queue;

  BallRenderer();

 public:
  BallRenderer(BallRenderer const&) = delete;
  void operator=(BallRenderer const&) = delete;
};
