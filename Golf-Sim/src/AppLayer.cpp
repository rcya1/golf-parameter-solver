#include "AppLayer.h"

#include <glm/gtx/rotate_vector.hpp>
#include <vector>

#include "GLCore/Core/KeyCodes.h"
#include "util/DebugColors.h"
#include "util/opengl/VertexArray.h"
#include "util/opengl/VertexBuffer.h"

using namespace GLCore;
using namespace GLCore::Utils;

AppLayer::AppLayer(GLFWwindow* window)
    : window(window),
      renderFrameBuffer(2000, 2000),
      cameraController(glm::vec3(0, 5.0, 5.0), -90, 0, 45.0, 2.0 / 1.0, 3.0,
                       5.0, 0.1),
      balls{Ball(-1.0, 5.0f, 0.0, 0.25, glm::vec3(0.808f, 0.471f, 0.408f)),
            Ball(1.0, 5.0f, 0.0, 0.25, glm::vec3(0.408f, 0.471f, 0.808f)),
            Ball(0.0, 0.0, 0.0, 0.25, glm::vec3(1.0f, 0.0f, 0.0f))},
      goal(0.5, 0.5, 0.5),
      terrain(glm::vec3(0.0, 0.0, 0.0), 100, 100, 50.0f, 50.0f, 10.0f, 5.0f),
      lightDepthShader("assets/shaders/LightDepthVertexShader.vert",
                       "assets/shaders/LightDepthFragmentShader.frag"),
      visualizeNormalsShader("assets/shaders/VisualizeNormals.vert",
                             "assets/shaders/VisualizeNormals.frag",
                             "assets/shaders/VisualizeNormals.geom"),
      primitiveShader("assets/shaders/PrimitiveShader.vert",
                      "assets/shaders/PrimitiveShader.frag"),
      lightDepthFrameBuffer0(1024, 1024),
      startPosition(0.2, 0.2),
      startPositionHighlightRadius(0.1),
      startPositionHighlightColor(1.0f, 0.843f, 0),
      addBallPosition(-1.0, 5.0f, 0.0),
      addBallRadius(0.25),
      addBallColor(0.808f, 0.471f, 0.408f) {
  lightScene = lights::LightScene{
      std::vector<lights::PointLight>{
          lights::createBasicPointLight(glm::vec3(25.0f, 25.0f, 25.0f)),
          lights::createBasicPointLight(glm::vec3(25.0f, 25.0f, -25.0f)),
          lights::createBasicPointLight(glm::vec3(-25.0f, 25.0f, 25.0f)),
          lights::createBasicPointLight(glm::vec3(-25.0f, 25.0f, -25.0f)),
          // lights::createBasicPointLight(glm::vec3(25.0f, -25.0f, 25.0f)),
          // lights::createBasicPointLight(glm::vec3(25.0f, -25.0f, -25.0f)),
          // lights::createBasicPointLight(glm::vec3(-25.0f, -25.0f, 25.0f)),
          // lights::createBasicPointLight(glm::vec3(-25.0f, -25.0f, -25.0f)),

          lights::createBasicPointLight(glm::vec3(0.0f, 20.0f, 0.0f)),
      },
      std::vector<lights::DirLight>{
          lights::createBasicDirLight(glm::vec3(0.0f, -1.0f, 0.0f)),
      }};
  lights::generateLightSpaceMatrices(lightScene);

  terrain.generateModel(goal);
  goal.generateModel(terrain);

  physicsWorld = physicsCommon.createPhysicsWorld();
  // physicsWorld->setNbIterationsVelocitySolver(15);
  // physicsWorld->setNbIterationsPositionSolver(8);

  terrain.addPhysics(physicsWorld, physicsCommon);
  goal.addPhysics(physicsWorld, physicsCommon);

  initializeBallsSimultaneous();
}

AppLayer::~AppLayer() {}

void AppLayer::OnAttach() {
  EnableGLDebugging();

  glEnable(GL_DEPTH_TEST);
  // doesn't really increase performance that much, but is useful for ensuring
  // consistent vertex ordering (for use with collision library)
  // glEnable(GL_CULL_FACE);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glEnable(GL_DEPTH_TEST);
  isCursorControllingCamera = true;
}

void AppLayer::OnDetach() {
  ballModel.freeModel();
  terrain.freeModel();
  goal.freeModel();

  ballRenderer.freeRenderer();
  terrainRenderer.freeRenderer();
  goalRenderer.freeRenderer();

  lightDepthFrameBuffer0.free();
  lightDepthShader.free();

  for (Ball& ball : balls) {
    ball.removePhysics(physicsWorld, physicsCommon);
  }
  terrain.removePhysics(physicsWorld, physicsCommon);
  goal.removePhysics(physicsWorld, physicsCommon);

  physicsCommon.destroyPhysicsWorld(physicsWorld);
}

void AppLayer::OnEvent(Event& event) {
  EventDispatcher dispatcher(event);
  dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e) {
    if (e.GetKeyCode() == HZ_KEY_ESCAPE) {
      if (isCursorControllingCamera) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        isCursorControllingCamera = false;
      } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        isCursorControllingCamera = true;
      }
    }

    return false;
  });
  dispatcher.Dispatch<GLCore::WindowResizeEvent>([&](WindowResizeEvent& e) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, e.GetWidth(), e.GetHeight());
    return false;
  });

  cameraController.OnEvent(event, isCursorControllingCamera);
}

void AppLayer::update(Timestep ts) {
  if (!justStartedPhysics && physicsRunning) {
    physicsAccumulatedTime += ts;
  }
  if (physicsRunning) {
    justStartedPhysics = false;
  }
  float desiredPhysicsTimeStep = 1.0 / 60.0f;
  while (physicsAccumulatedTime >= desiredPhysicsTimeStep && physicsRunning) {
    physicsAccumulatedTime -= desiredPhysicsTimeStep;
    physicsWorld->update(desiredPhysicsTimeStep);
  }

  cameraController.update(ts);

  float interpolationFactor =
      physicsRunning ? physicsAccumulatedTime / desiredPhysicsTimeStep : 0;
  while (!ballsAdd.empty()) {
    balls.push_back(ballsAdd.front());
    ballsAdd.pop();
  }
  for (Ball& ball : balls) {
    ball.update(ts, terrain, goal, physicsWorld, physicsCommon,
                interpolationFactor);
  }
  terrain.update(ts, interpolationFactor);

  render();

  timeMetrics.update(ts);
}

void AppLayer::render() {
  if (renderPhysicsDebugging) {
    renderFrameBuffer.prepareForRender();

    physicsWorld->setIsDebugRenderingEnabled(true);
    reactphysics3d::DebugRenderer& debugRenderer =
        physicsWorld->getDebugRenderer();
    debugRenderer.setIsDebugItemDisplayed(
        reactphysics3d::DebugRenderer::DebugItem::COLLIDER_AABB, true);
    // debugRenderer.setIsDebugItemDisplayed(
    //     reactphysics3d::DebugRenderer::DebugItem::COLLISION_SHAPE, true);

    // debugRenderer.setIsDebugItemDisplayed(
    //     reactphysics3d::DebugRenderer::DebugItem::CONTACT_NORMAL, true);
    // debugRenderer.setIsDebugItemDisplayed(
    //     reactphysics3d::DebugRenderer::DebugItem::CONTACT_POINT, true);

    debugRenderer.computeDebugRenderingPrimitives(*physicsWorld);

    primitiveShader.activate();
    primitiveShader.setMat4f("view", false,
                             cameraController.getCamera().getViewMatrix());
    primitiveShader.setMat4f(
        "projection", false,
        cameraController.getCamera().getProjectionMatrix());

    if (debugRenderer.getNbLines() > 0) {
      const reactphysics3d::DebugRenderer::DebugLine* linesArray =
          debugRenderer.getLinesArray();

      std::vector<float> vertexData;
      for (int i = 0; i < debugRenderer.getNbLines(); i++) {
        vertexData.push_back(linesArray[i].point1.x);
        vertexData.push_back(linesArray[i].point1.y);
        vertexData.push_back(linesArray[i].point1.z);

        glm::vec3 color1 = convertColor(linesArray[i].color1);
        vertexData.push_back(color1.x);
        vertexData.push_back(color1.y);
        vertexData.push_back(color1.z);

        vertexData.push_back(linesArray[i].point2.x);
        vertexData.push_back(linesArray[i].point2.y);
        vertexData.push_back(linesArray[i].point2.z);

        glm::vec3 color2 = convertColor(linesArray[i].color2);
        vertexData.push_back(color2.x);
        vertexData.push_back(color2.y);
        vertexData.push_back(color2.z);
      }

      opengl::VertexArray vertexArray;
      vertexArray.bind();
      opengl::VertexBuffer vertexBuffer(vertexData.size() * sizeof(float),
                                        vertexData.data(), 6 * sizeof(float),
                                        GL_STATIC_DRAW);
      vertexBuffer.bind();
      vertexBuffer.setVertexAttribute(0, 3, GL_FLOAT, 0);
      vertexBuffer.setVertexAttribute(1, 3, GL_FLOAT, 3 * sizeof(float));

      glDrawArrays(GL_LINES, 0, debugRenderer.getNbLines() * 2);
    }

    if (debugRenderer.getNbTriangles() > 0) {
      const reactphysics3d::DebugRenderer::DebugTriangle* trianglesArray =
          debugRenderer.getTrianglesArray();

      std::vector<float> vertexData;
      for (int i = 0; i < debugRenderer.getNbTriangles(); i++) {
        vertexData.push_back(trianglesArray[i].point1.x);
        vertexData.push_back(trianglesArray[i].point1.y);
        vertexData.push_back(trianglesArray[i].point1.z);

        glm::vec3 color1 = convertColor(trianglesArray[i].color1);
        vertexData.push_back(color1.x);
        vertexData.push_back(color1.y);
        vertexData.push_back(color1.z);

        vertexData.push_back(trianglesArray[i].point2.x);
        vertexData.push_back(trianglesArray[i].point2.y);
        vertexData.push_back(trianglesArray[i].point2.z);

        glm::vec3 color2 = convertColor(trianglesArray[i].color2);
        vertexData.push_back(color2.x);
        vertexData.push_back(color2.y);
        vertexData.push_back(color2.z);

        vertexData.push_back(trianglesArray[i].point3.x);
        vertexData.push_back(trianglesArray[i].point3.y);
        vertexData.push_back(trianglesArray[i].point3.z);

        glm::vec3 color3 = convertColor(trianglesArray[i].color3);
        vertexData.push_back(color3.x);
        vertexData.push_back(color3.y);
        vertexData.push_back(color3.z);
      }

      opengl::VertexArray vertexArray;
      vertexArray.bind();

      opengl::VertexBuffer vertexBuffer(vertexData.size() * sizeof(float),
                                        vertexData.data(), 6 * sizeof(float),
                                        GL_STATIC_DRAW);
      vertexBuffer.bind();
      vertexBuffer.setVertexAttribute(0, 3, GL_FLOAT, 0);
      vertexBuffer.setVertexAttribute(1, 3, GL_FLOAT, 3 * sizeof(float));

      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDrawArrays(GL_TRIANGLES, 0, debugRenderer.getNbTriangles() * 3);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  } else {
    physicsWorld->setIsDebugRenderingEnabled(false);

    lightDepthFrameBuffer0.prepareForCalculate();
    if (renderShadows) {
      for (Ball& ball : balls) {
        ball.render(ballRenderer);
      }

      ballRenderer.renderLightDepth(ballModel, lightDepthShader, lightScene, 0);
    }
    lightDepthFrameBuffer0.unbind();

    renderFrameBuffer.prepareForRender();

    if (renderNormals) {
      // goal.render(goalRenderer);
      // goalRenderer.render(cameraController.getCamera(), lightScene,
      //                     &visualizeNormalsShader);
      terrain.render(terrainRenderer, startPosition,
                     startPositionHighlightRadius, startPositionHighlightColor);
      terrainRenderer.render(cameraController.getCamera(), lightScene,
                             &visualizeNormalsShader);
    }

    for (Ball& ball : balls) {
      ball.render(ballRenderer);
    }
    terrain.render(terrainRenderer, startPosition, startPositionHighlightRadius,
                   startPositionHighlightColor);
    goal.render(goalRenderer);

    lightDepthFrameBuffer0.bindAsTexture();
    ballRenderer.render(ballModel, cameraController.getCamera(), lightScene);
    terrainRenderer.render(cameraController.getCamera(), lightScene);
    goalRenderer.render(cameraController.getCamera(), lightScene);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  int windowWidth, windowHeight;
  glfwGetWindowSize(window, &windowWidth, &windowHeight);
  glViewport(0, 0, windowWidth, windowHeight);
}

void AppLayer::imGuiRender() {
  ImGui::DockSpaceOverViewport();

  ImGui::Begin("Viewport");
  ImVec2 windowSize = ImGui::GetContentRegionAvail();
  ImGui::BeginChild("Render");
  renderFrameBuffer.updateSize(windowSize.x, windowSize.y);
  cameraController.updateSize(windowSize.x, windowSize.y);
  ImGui::Image((ImTextureID)renderFrameBuffer.textureId, windowSize,
               ImVec2(0, 1), ImVec2(1, 0));
  ImGui::EndChild();
  ImGui::End();

  ImGui::Begin("Simulation Settings");
  if (ImGui::Button("Start / Stop")) {
    physicsRunning = !physicsRunning;
    if (physicsRunning) {
      justStartedPhysics = true;
    }
  }
  if (ImGui::Button("Reset Balls (Simultaneous)")) {
    initializeBallsSimultaneous();
  }
  if (ImGui::Button("Reset Balls (Staggered)")) {
    initializeBallsStaggered();
  }
  ImGui::DragFloat2("Start Position", glm::value_ptr(startPosition), 0.05f,
                    0.0f, 0.25f);
  ImGui::DragFloat("Highlight Radius", &startPositionHighlightRadius, 0.01f,
                   0.0f, 1.0f);
  ImGui::ColorEdit3("Highlight Color",
                    glm::value_ptr(startPositionHighlightColor));
  ImGui::DragInt("# Balls per Dimension", &paramsNumDivisions, 0.25, 1, 15);
  ImGui::DragFloatRange2("Power", &minPower, &maxPower, 0.25f, 0.0f, 30.0);
  ImGui::DragFloatRange2("Yaw", &minYaw, &maxYaw, 0.25f, -PI / 2, PI / 2);
  ImGui::DragFloatRange2("Pitch", &minPitch, &maxPitch, 0.25f, 0.0f, PI / 2);

  ImGui::End();

  ImGui::Begin("Balls");
  for (int i = 0; i < balls.size(); i++) {
    balls[i].imGuiRender(i, physicsWorld, physicsCommon);
  }
  ImGui::Begin("Add Ball");
  ImGui::DragFloat3("Position", glm::value_ptr(addBallPosition), 1.0f, -10.0f,
                    10.0f);
  ImGui::DragFloat("Radius", &addBallRadius, 0.01f, 0.01f, 2.0f);
  ImGui::ColorEdit3("Color", glm::value_ptr(addBallColor));
  ImGui::Checkbox("Has Physics", &addBallHasPhysics);
  if (ImGui::Button("Add")) {
    Ball ball = Ball(addBallPosition.x, addBallPosition.y, addBallPosition.z,
                     addBallRadius, addBallColor);
    if (addBallHasPhysics) {
      ball.addPhysics(physicsWorld, physicsCommon);
    }
    ballsAdd.push(ball);
  }
  ImGui::End();
  ImGui::End();

  terrain.imGuiRender(goal, physicsWorld, physicsCommon);
  goal.imGuiRender(physicsWorld, physicsCommon, terrain);
  timeMetrics.imGuiRender();

  ImGui::Begin("Rendering");
  ImGui::Checkbox("Shadows", &renderShadows);
  ImGui::Checkbox("Normals", &renderNormals);
  ImGui::Checkbox("Physics Debugging", &renderPhysicsDebugging);
  if (ImGui::Button("Reload Shaders")) {
    lightDepthShader.load();
    ballRenderer.reloadShader();
    goalRenderer.reloadShader();
    terrainRenderer.reloadShader();
  }
  ImGui::End();
}

void AppLayer::initializeBallsSimultaneous() {
  for (Ball& ball : balls) {
    ball.removePhysics(physicsWorld, physicsCommon);
  }
  balls.clear();

  const float POWER_DIV = (maxPower - minPower) / (paramsNumDivisions - 1);
  const float YAW_OFFSET_DIV = (maxYaw - minYaw) / (paramsNumDivisions - 1);
  const float PITCH_DIV = (maxPitch - minPitch) / (paramsNumDivisions - 1);

  for (int i = 0; i < paramsNumDivisions; i++) {
    float power = minPower + POWER_DIV * i;
    for (int j = 0; j < paramsNumDivisions; j++) {
      float yawOffset = minYaw + YAW_OFFSET_DIV * j;
      for (int k = 0; k < paramsNumDivisions; k++) {
        float pitch = minPitch + PITCH_DIV * k;
        addBall(power, yawOffset, pitch);
      }
    }
  }
}

void AppLayer::initializeBallsStaggered() {}

void AppLayer::addBall(float power, float yawOffset, float pitch) {
  glm::vec2 startPositionAbs = terrain.convertUV(startPosition);
  glm::vec2 dirVector =
      glm::normalize(goal.getAbsolutePosition(terrain) - startPositionAbs);
  glm::vec2 perpVector(-dirVector.y, dirVector.x);

  glm::vec2 rotatedYawDir = glm::rotate(dirVector, yawOffset);
  glm::vec2 rotatedPerp = glm::rotate(perpVector, yawOffset);
  glm::vec3 rotatedDir =
      glm::rotate(glm::vec3(rotatedYawDir.x, 0, rotatedYawDir.y), pitch,
                  glm::vec3(rotatedPerp.x, 0, rotatedPerp.y));
  glm::vec3 finalDir = rotatedDir * power;

  Ball ball(startPositionAbs.x,
            terrain.getHeightFromRelative(
                glm::vec2(startPosition.x * terrain.getWidth(),
                          startPosition.y * terrain.getHeight())) +
                terrain.getPosition().y + addBallRadius * 2,
            startPositionAbs.y, addBallRadius, addBallColor);
  ball.addPhysics(physicsWorld, physicsCommon);
  ball.setVelocity(finalDir);

  balls.push_back(ball);
}
