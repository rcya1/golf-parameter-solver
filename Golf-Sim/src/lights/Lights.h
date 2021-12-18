#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "util/opengl/Shader.h"

namespace lights {
struct PointLight {
  glm::vec3 position;

  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

struct DirLight {
  glm::vec3 direction;
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
};

struct LightScene {
  std::vector<PointLight> pointLights;
  std::vector<DirLight> dirLights;
};

void setLightScene(opengl::Shader& shader, LightScene& lightScene);
PointLight createBasicPointLight(glm::vec3 position);
DirLight createBasicDirLight(glm::vec3 direction);
};  // namespace lights
