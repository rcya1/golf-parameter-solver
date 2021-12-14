#include "Lights.h"

void lights::setLightScene(opengl::Shader& shader,
                           lights::LightScene& lightScene) {
  shader.setInt("numPointLights", lightScene.pointLights.size());
  for (int i = 0; i < lightScene.pointLights.size(); i++) {
    lights::PointLight& pointLight = lightScene.pointLights[i];
    std::string prefix =
        std::string("pointLights[") + std::to_string(i) + std::string("]");
    shader.setVec3f((prefix + ".position").c_str(), pointLight.position);
    shader.setVec3f((prefix + ".ambient").c_str(), pointLight.ambient);
    shader.setVec3f((prefix + ".diffuse").c_str(), pointLight.diffuse);
    shader.setVec3f((prefix + ".specular").c_str(), pointLight.specular);
    shader.setFloat((prefix + ".constant").c_str(), pointLight.constant);
    shader.setFloat((prefix + ".linear").c_str(), pointLight.linear);
    shader.setFloat((prefix + ".quadratic").c_str(), pointLight.quadratic);
  }

  shader.setInt("numDirLights", lightScene.dirLights.size());
  for (int i = 0; i < lightScene.dirLights.size(); i++) {
    DirLight& dirLight = lightScene.dirLights[i];
    std::string prefix =
        std::string("dirLights[") + std::to_string(i) + std::string("]");
    shader.setVec3f((prefix + ".direction").c_str(), dirLight.direction);
    shader.setVec3f((prefix + ".ambient").c_str(), dirLight.ambient);
    shader.setVec3f((prefix + ".diffuse").c_str(), dirLight.diffuse);
    shader.setVec3f((prefix + ".specular").c_str(), dirLight.specular);
  }
}

lights::PointLight lights::createBasicPointLight(glm::vec3 position) {
  return PointLight{position,
                    glm::vec3(0.05f, 0.05f, 0.05f),
                    glm::vec3(1.0f, 1.0f, 1.0f),
                    glm::vec3(1.0f, 1.0f, 1.0f),
                    1.0f,
                    0.022f,
                    0.0019f};
}

lights::DirLight lights::createBasicDirLight(glm::vec3 direction) {
  return DirLight{direction, glm::vec3(1.0f, 1.0f, 1.0f),
                  glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f)};
}
