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

    for (int i = 0; i < lightScene.dirLights.size(); i++) {
      shader.setMat4f(("lightSpaceMatrix[" + std::to_string(i) + "]").c_str(),
                      false,
                      glm::value_ptr(lightScene.dirLights[i].lightSpaceMatrix));
    }
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
                  glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::mat4()};
}

void lights::generateLightSpaceMatrices(LightScene& lightScene) {
  for (DirLight& dirLight : lightScene.dirLights) {
    glm::vec3 direction = dirLight.direction;
    dirLight.lightSpaceMatrix =
        glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f) *
        glm::lookAt(-direction, glm::vec3(0),
                    glm::cross(direction, glm::vec3(-1.0f, 0.0f, 0.0f)));
  }
}
