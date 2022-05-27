#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace opengl {
class Shader;
}

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
  glm::mat4 lightSpaceMatrix;  // can leave empty and then call
                               // generateLightSpaceMatrices
};

struct LightScene {
  std::vector<PointLight> pointLights;
  std::vector<DirLight> dirLights;
};

void setLightScene(opengl::Shader* shader, LightScene& lightScene);
PointLight createBasicPointLight(glm::vec3 position);
DirLight createBasicDirLight(glm::vec3 direction);
void generateLightSpaceMatrices(LightScene& lightScene);
};  // namespace lights
