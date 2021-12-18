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
    : shader("assets/shaders/LightingVertexShader.vert",
             "assets/shaders/LightingFragmentShader.frag") {
  BallModel::getInstance();  // make sure the model is loaded
}

void BallRenderer::render(opengl::PerspectiveCamera& camera,
                          lights::LightScene& lightScene) {
  shader.activate();
  shader.setVec3f("material.ambient", 0.0f, 0.0f, 0.0f);
  shader.setVec3f("material.specular", 0.25f, 0.25f, 0.25f);
  shader.setFloat("material.shininess", 2);
  shader.setVec3f("viewPos", camera.getPos());
  shader.setMat4f("view", false, camera.getViewMatrix());
  shader.setMat4f("projection", false, camera.getProjectionMatrix());

  lights::setLightScene(shader, lightScene);

  BallModel::getInstance().getVertexArray()->bind();

  while (queue.size()) {
    BallRenderJob job = queue.front();

    queue.pop();

    shader.setMat4f("model", false, glm::value_ptr(job.model));
    shader.setVec3f("material.diffuse", job.color);
    glDrawElements(GL_TRIANGLES, BallModel::getInstance().getIndexDataSize(),
                   GL_UNSIGNED_INT, (void*)0);
  }
}

void BallRenderer::add(BallRenderJob job) { queue.push(job); }

void BallRenderer::freeRenderer() { BallModel::getInstance().freeModel(); }