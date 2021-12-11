#pragma once

#include "util/Shader.h"
#include "util/PerspectiveCamera.h"
#include "lights/Lights.h"

#include <GLCoreUtils.h>
#include <glm/glm.hpp>
#include <queue>

class BallRenderer {
public:
  static BallRenderer& getInstance() {
    static BallRenderer instance;
    return instance;
  }

  void render(PerspectiveCamera& camera, lights::LightScene& lightScene);
  void add(glm::mat4 model);
  void freeRenderer();

private:
  Shader shader;
  std::queue<glm::mat4> queue;

  BallRenderer();

public:
  BallRenderer(BallRenderer const&) = delete;
  void operator=(BallRenderer const&) = delete;
};

