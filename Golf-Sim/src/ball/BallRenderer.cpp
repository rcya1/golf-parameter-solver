#include "BallRenderer.h"

#include <GLCoreUtils.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "BallModel.h"
#include "lights/Lights.h"
#include "util/opengl/PerspectiveCamera.h"

BallRenderer::BallRenderer()
    : shader("assets/shaders/BallVertexShader.vert",
             "assets/shaders/BallFragmentShader.frag") {}

void BallRenderer::render(BallModel& model, opengl::PerspectiveCamera& camera,
                          lights::LightScene& lightScene) {
  shader.activate();
  shader.setVec3f("material.ambient", 0.0f, 0.0f, 0.0f);
  shader.setVec3f("material.specular", 0.25f, 0.25f, 0.25f);
  shader.setFloat("material.shininess", 2);
  shader.setVec3f("viewPos", camera.getPos());
  shader.setMat4f("view", false, camera.getViewMatrix());
  shader.setMat4f("projection", false, camera.getProjectionMatrix());

  lights::setLightScene(shader, lightScene);

  model.getVertexArray()->bind();

  while (queue.size()) {
    BallRenderJob job = queue.front();
    queue.pop();

    shader.setMat4f("model", false, glm::value_ptr(job.model));
    shader.setVec3f("material.diffuse", job.color);
    glDrawElements(GL_TRIANGLES, model.getIndexDataSize(), GL_UNSIGNED_INT,
                   (void*)0);
  }
}

void BallRenderer::renderLightDepth(BallModel& model,
                                    opengl::Shader& lightDepthShader,
                                    lights::LightScene& lightScene,
                                    int dirLightIndex) {
  lightDepthShader.activate();
  lightDepthShader.setMat4f(
      "lightSpaceMatrix", false,
      glm::value_ptr(lightScene.dirLights[dirLightIndex].lightSpaceMatrix));

  model.getVertexArray()->bind();

  while (queue.size()) {
    BallRenderJob job = queue.front();
    queue.pop();

    lightDepthShader.setMat4f("model", false, glm::value_ptr(job.model));
    glDrawElements(GL_TRIANGLES, model.getIndexDataSize(), GL_UNSIGNED_INT,
                   (void*)0);
  }
}

void BallRenderer::add(BallRenderJob job) { queue.push(job); }

void BallRenderer::freeRenderer() { shader.free(); }