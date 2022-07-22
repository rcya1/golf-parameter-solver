#include "AppLayer.h"

#include <filesystem>
#include <fstream>
#include <glm/gtx/rotate_vector.hpp>
#include <vector>

#include "GLCore/Core/KeyCodes.h"
#include "IconsFontAwesome.h"
#include "ImGuiConstants.h"
#include "ImGuiFileDialog.h"
#include "backends/imgui_impl_opengl3.h"
#include "util/DebugColors.h"
#include "util/opengl/VertexArray.h"
#include "util/opengl/VertexBuffer.h"

using namespace GLCore;
using namespace GLCore::Utils;

AppLayer::AppLayer(GLFWwindow* window)
    : window(window),
      renderFrameBuffer(10000, 10000),
      cameraController(glm::vec3(0, 5.0, 5.0), -90, 0, 45.0, 2.0 / 1.0, 5.0,
                       5.0, 0.1),
      balls{},
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
}

AppLayer::~AppLayer() {}

void AppLayer::OnAttach() {
  EnableGLDebugging();

  glEnable(GL_DEPTH_TEST);
  // doesn't really increase performance that much, but is useful for ensuring
  // consistent vertex ordering (for use with collision library)
  glEnable(GL_CULL_FACE);

  glEnable(GL_DEPTH_TEST);

  ImGui::GetIO().Fonts->ClearFonts();
  ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Medium.ttf",
                                           13);
  ImFontConfig config;
  config.MergeMode = true;
  config.GlyphMinAdvanceX = 13.0f;
  static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
  ImGui::GetIO().Fonts->AddFontFromFileTTF(
      "assets/fonts/" FONT_ICON_FILE_NAME_FAS, 13.0f, &config, icon_ranges);
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
    ball.removePhysics(physicsWorld, physicsCommon, ballShapeRegistry);
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
  // check if the staggered initialization is ready for next batch
  if (staggeredBalls.size() > 0) {
    bool anyActive = false;
    for (Ball& ball : balls) {
      if (ball.getState() == BallState::ACTIVE) {
        anyActive = true;
      }
    }

    if (!anyActive) {
      for (Ball& ball : balls) {
        ball.removePhysics(physicsWorld, physicsCommon, ballShapeRegistry);
      }

      int initSize = staggeredBalls.size();
      for (int i = initSize - 1;
           i >= std::max(0, initSize - staggeredBatchSize); i--) {
        addBall(staggeredBalls[i]);
        staggeredBalls.pop_back();
      }
    }
  }

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
                ballShapeRegistry, interpolationFactor);
  }
  terrain.update(ts, interpolationFactor);

  render();

  timeMetrics.update(ts);

  exportReady = false;
  if (paramsNumDivisions * paramsNumDivisions * paramsNumDivisions ==
      balls.size()) {
    bool ready = true;
    for (Ball& ball : balls) {
      if (ball.getState() == BallState::ACTIVE) {
        ready = false;
        break;
      }
    }

    if (ready) {
      exportReady = true;
    }
  }
  exportReady = true;

  // update font if the DPI scale has changed
  if (updateFont) {
    ImGui::GetIO().Fonts->Clear();
    ImGui::GetIO().Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Medium.ttf",
                                             static_cast<int>(13 * dpiScale));
    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = static_cast<int>(13 * dpiScale);
    static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImGui::GetIO().Fonts->AddFontFromFileTTF(
        "assets/fonts/" FONT_ICON_FILE_NAME_FAS, 13 * dpiScale, &config,
        icon_ranges);

    ImGui::GetIO().Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    updateFont = false;
  }
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

    debugRenderer.setIsDebugItemDisplayed(
        reactphysics3d::DebugRenderer::DebugItem::CONTACT_NORMAL, true);
    // debugRenderer.setIsDebugItemDisplayed(
    //     reactphysics3d::DebugRenderer::DebugItem::CONTACT_POINT, true);
    debugRenderer.setContactPointSphereRadius(0.1);
    debugRenderer.setContactNormalLength(1.0);

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

      // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glDrawArrays(GL_TRIANGLES, 0, debugRenderer.getNbTriangles() * 3);
      // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      debugRenderer.reset();
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
      goal.render(goalRenderer);
      goalRenderer.render(cameraController.getCamera(), lightScene,
                          &visualizeNormalsShader);
      // terrain.render(terrainRenderer, startPosition,
      //                startPositionHighlightRadius,
      //                startPositionHighlightColor);
      // terrainRenderer.render(cameraController.getCamera(), lightScene,
      //                        &visualizeNormalsShader);
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

  // prepare render for ImGui
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  int windowWidth, windowHeight;
  glfwGetWindowSize(window, &windowWidth, &windowHeight);
  glViewport(0, 0, windowWidth, windowHeight);
}

inline void SetupImGuiStyle() {
  ImGui::GetStyle().FrameRounding = 4.0f;
  ImGui::GetStyle().GrabRounding = 4.0f;

  ImVec4* colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "",
                                            ImVec4(0.2f, 1.0f, 1.0f, 1.0f),
                                            ICON_FA_FOLDER);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".golf",
                                            ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                                            ICON_FA_GOLF_BALL_TEE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "",
                                            ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                                            ICON_FA_FILE);
}

void pushActiveTabColors() {
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.63f, 1.00f, 1.00f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        ImVec4(0.38f, 0.66f, 1.00f, 1.00f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        ImVec4(0.16f, 0.63f, 1.00f, 1.00f));
}

void renderHelpMenu() {
  static int currHelpTab = 0;
  
  bool pushedColors = false;

  std::map<int, std::string> tabMapping = {
    std::make_pair(0, "Overview"),
    std::make_pair(1, "Controls"),
    std::make_pair(2, "Running Simulation"),
    std::make_pair(3, "Exporting Results")
  };

  for (auto& x : tabMapping) {
    int tab = x.first;
    std::string tabName = x.second;

    ImGui::SameLine();
    bool activeTab = (currHelpTab == tab);
    if (activeTab) {
      pushActiveTabColors();
    }
    if (ImGui::Button(tabName.c_str())) {
      currHelpTab = tab;
    }
    if (activeTab) {
      ImGui::PopStyleColor(3);
    }
  }
  
  switch(currHelpTab) {
    case 0:
      ImGui::Spacing();

      ImGui::TextWrapped("Golf Simulator allows you to hundreds of thousands of random golf shots "
        "on randomly generated terrain");

      ImGui::Spacing();

      ImGui::TextWrapped("All of the simulation controls are displayed on the left. You can modify the "
          "parameters that are shot, the number of balls shot, and the "
          "progress through the simulation.");

      ImGui::Spacing();

      ImGui::TextWrapped("To change parameters such as the ball color / size or terrain / goal generation, "
        "you can turn panels on containing those settings through the menu bar under 'Settings'. For most settings "
        "to change, you need to either regenerate the balls (through pressing 'Init Balls') or by pushing the 'Generate' "
          "button at the bottom of the respective settings pane.");

      ImGui::Spacing();

      ImGui::TextWrapped("For information on how to fly through the simulation, you can go to the 'Visualization "
          "Controls' tab.");

      ImGui::Spacing();

      ImGui::TextWrapped("For information on how to run the simulation, you can go to the 'Running "
          "Simulation' tab.");

      ImGui::Spacing();

      ImGui::TextWrapped("For information on how to export results for visualization, you can go to the 'Exporting "
        "Results' tab.");
      break;
    case 1:
      ImGui::Spacing();

      ImGui::TextWrapped("To swap being able to use your cursor to click on the GUI and your cursor being used to "
        "pan around the simulation, press Escape");

      ImGui::Spacing();

      ImGui::TextWrapped("While in control of the simulation, you can use WASD to move back and forth at the same "
        "height. Space is used to ascend, and Shift is used to descend.");

      ImGui::Spacing();

      ImGui::TextWrapped("Use the mouse wheel to change the FOV, effectively zooming in (warning: this causes "
        "distortion, especially at extreme FOVs.");

      break;
    case 2:
      ImGui::Spacing();
 
      ImGui::TextWrapped("Running Simulation");
      ImGui::TextWrapped("Every shot in the simulation is defined by three parameters, its power (initial velocity), "
        "pitch (angle from horizontal), and yaw offset (angle offset from aimed directly toward the goal).");

      ImGui::Spacing();

      ImGui::TextWrapped("For the simulation, you specify a range for each of these three parameters as well as "
        "the number of divisions for each parameter. For instance, if the number of divisions is set to 5, then each "
        "parameter range is divided into 5 equally spaced value, and for each of the 5^3 = 125 combinations of parameters, "
        "a single golf ball is launched.");

      ImGui::Spacing();

      ImGui::TextWrapped("After specifying these parameters, there are two options for starting the simulation: "
        "staggered and simultaneous. For simultaneous starts, all balls are launched at the same time, which is highly "
        "unrecommended for more than 200 balls due to performance reasons. For staggered starts, the balls are launched in "
        "batches, and this batch size can be configured while the staggered option is selected. By default, the staggered "
        "option is selected.");

      break;
    case 3:
      ImGui::Spacing();

      ImGui::TextWrapped("After the simulation finishes running, the 'Export Results' button shows up, "
        "allowing you to save the results of each ball to a file.");

      ImGui::Spacing();

      ImGui::TextWrapped("Upon pressing the export button, a file dialog appears where you can select the folder "
        "the results file will be saved to, as well as the name of the file.");

      ImGui::Spacing();

      ImGui::TextWrapped("By default, the file will be saved with the .golf extension, indicating that it is an "
        "output of Golf Simulator.");
      break;
  }
}

void AppLayer::imGuiRender() {
  SetupImGuiStyle();

  ImVec2 windowSize = ImGui::GetWindowSize();

  if (dpiScale != ImGui::GetWindowDpiScale() &&
      ImGui::GetWindowDpiScale() != 0.0) {
    dpiScale = ImGui::GetWindowDpiScale();
    updateFont = true;
  }

  bool showRenderMenu = false;

  if (showHelpPopup) {
    if (!ImGui::Begin("Help", &showHelpPopup, ImGuiWindowFlags_NoDocking)) {
      // blank
    }
    else {
      renderHelpMenu();
    }
    ImGui::End();
  }

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::Button("Show Help")) {
      showHelpPopup = true;
    }
    ImGui::Separator();
    
    if (ImGui::BeginMenu("Settings")) {
      ImGui::Checkbox("Sidebar", &showSidebar);
      ImGui::Checkbox("Ball Settings", &showBallSettings);
      ImGui::Checkbox("Terrain Settings", &showTerrainSettings);
      ImGui::Checkbox("Goal Settings", &showGoalSettings);
      ImGui::EndMenu();
    }
    ImGui::Separator();
    
    if (ImGui::BeginMenu("Debugging")) {
      ImGui::Checkbox("Show Extra Debugging Windows", &showDebugWindows);
      ImGui::Checkbox("Shadows", &renderShadows);
      ImGui::Checkbox("Normals", &renderNormals);
      ImGui::Checkbox("Physics Debugging", &renderPhysicsDebugging);
      ImGui::Checkbox("Show Time Metrics", &showTimeMetrics);

      if (ImGui::Button("Reload Shaders")) {
        lightDepthShader.load();
        ballRenderer.reloadShader();
        goalRenderer.reloadShader();
        terrainRenderer.reloadShader();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  ImGui::DockSpaceOverViewport();

  ImGui::Begin("Viewport", NULL,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
  ImVec2 viewportSize = ImGui::GetContentRegionAvail();
  ImGui::BeginChild("Render");
  renderFrameBuffer.updateSize(viewportSize.x, viewportSize.y);
  cameraController.updateSize(viewportSize.x, viewportSize.y);
  ImGui::Image((ImTextureID)renderFrameBuffer.textureId, viewportSize,
               ImVec2(0, 1), ImVec2(1, 0));
  ImGui::EndChild();
  ImGui::End();

  if (showSidebar) {
    ImGui::Begin("Simulation", NULL, ImGuiWindowFlags_NoMove);
    if (ImGui::Checkbox("Physics Running", &physicsRunning)) {
      if (physicsRunning) {
        justStartedPhysics = true;
      }
    }

    setupGreenButton();
    if (ImGui::Button("Init Balls")) {
      initializeBalls(!initSimultaneous);
    }
    clearButtonStyle();

    ImGui::SameLine();
    if (ImGui::RadioButton("Staggered", !initSimultaneous)) {
      initSimultaneous = false;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Simultaneous", initSimultaneous)) {
      initSimultaneous = true;
    }
    setupGreenButton();
    if (exportReady) {
      if (ImGui::Button(ICON_FA_FLOPPY_DISK " Export Results")) {
        const char* filters = "Results File (*.golf){.golf}";
        ImGuiFileDialog::Instance()->OpenDialog(
            "Export", ICON_FA_FLOPPY_DISK " Select Output File", filters, ".",
            "result", 1, IGFDUserDatas("SaveFile"),
            ImGuiFileDialogFlags_ConfirmOverwrite);
      }
    }
    clearButtonStyle();

    ImVec2 maxSize = windowSize;
    ImVec2 minSize = ImVec2(windowSize.x / 2.0, windowSize.y / 2.0);
    if (ImGuiFileDialog::Instance()->Display(
            "Export", ImGuiWindowFlags_NoDocking, minSize, maxSize)) {
      if (ImGuiFileDialog::Instance()->IsOk()) {
        outputFilePath =
            ImGuiFileDialog::Instance()->GetFilePathName() + ".golf";
        std::cout << outputFilePath << std::endl;
        std::ofstream fout(outputFilePath);
        writeOutputFile(fout);
        fout.close();
      }

      ImGuiFileDialog::Instance()->Close();
    }

    if (!initSimultaneous) {
      ImGui::NewLine();

      ImGui::DragInt("Staggered Batch Size", &staggeredBatchSize, 5.0f, 1, 500);
    }

    if (staggeredBalls.empty()) {
      ImGui::NewLine();

      ImGui::DragFloat2("Start Position", glm::value_ptr(startPosition), 0.05f,
                        0.0f, 0.25f);
      ImGui::DragFloat("Highlight Radius", &startPositionHighlightRadius, 0.01f,
                       0.0f, 1.0f);
      ImGui::ColorEdit3("Highlight Color",
                        glm::value_ptr(startPositionHighlightColor));
      ImGui::NewLine();

      ImGui::DragInt("# Balls per Dim", &paramsNumDivisions, 0.25, 1, 100);
      ImGui::DragFloatRange2("Power", &minPower, &maxPower, 0.25f, 0.0f, 30.0);
      ImGui::DragFloatRange2("Yaw Offset (deg)", &minYaw, &maxYaw, 1.5f, -90.0f,
                             90.0f);
      ImGui::DragFloatRange2("Pitch (deg)", &minPitch, &maxPitch, 1.0f, 0.0f,
                             90.0f);
    } else {
      ImGui::Text("%d Balls Left", staggeredBalls.size());
      setupRedButton();
      if (ImGui::Button("Cancel Staggered")) {
        staggeredBalls.clear();
        for (Ball& ball : balls) {
          ball.removePhysics(physicsWorld, physicsCommon, ballShapeRegistry);
        } 
        balls.clear();
      }
      clearButtonStyle();
    }

    ImGui::End();

    if (showDebugWindows) {
      ImGui::Begin("Balls", NULL, ImGuiWindowFlags_NoMove);
      for (int i = 0; i < balls.size(); i++) {
        balls[i].imGuiRender(i, physicsWorld, physicsCommon, ballShapeRegistry);
      }
      ImGui::End();

      float bound = std::max(terrain.getWidth(), terrain.getHeight()) / 2;
      ImGui::DragFloat3("New Ball Pos", glm::value_ptr(addBallPosition), 1.0f,
                        -bound / 2, bound / 2);
      ImGui::Checkbox("New Ball Has Physics", &addBallHasPhysics);

      setupGreenButton();
      if (ImGui::Button("Add")) {
        Ball ball = Ball(addBallPosition.x, addBallPosition.y,
                         addBallPosition.z, addBallRadius, addBallColor);
        if (addBallHasPhysics) {
          ball.addPhysics(physicsWorld, physicsCommon, ballShapeRegistry);
        }
        ballsAdd.push(ball);
      }
      clearButtonStyle();
    }

    if (showBallSettings) {
      ImGui::Begin("Ball Settings", NULL, ImGuiWindowFlags_NoMove);
      ImGui::DragFloat("Radius", &addBallRadius, 0.01f, 0.01f, 2.0f);
      ImGui::ColorEdit3("Color", glm::value_ptr(addBallColor));
      ImGui::End();
    }

    if (showTerrainSettings) {
      ImGui::Begin("Terrain Controls", NULL, ImGuiWindowFlags_NoMove);
      terrain.imGuiRender(goal, physicsWorld, physicsCommon);
      ImGui::End();
    }

    if (showGoalSettings) {
      ImGui::Begin("Goal Controls", NULL, ImGuiWindowFlags_NoMove);
      goal.imGuiRender(physicsWorld, physicsCommon, terrain);
      ImGui::End();
    }
  }

  if (showTimeMetrics) {
    ImGui::Begin("Time Metrics");
    timeMetrics.imGuiRender(dpiScale);
    ImGui::End();
  }
}

void AppLayer::initializeBalls(bool staggered) {
  for (Ball& ball : balls) {
    ball.removePhysics(physicsWorld, physicsCommon, ballShapeRegistry);
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
        addBall(power, yawOffset * PI / 180, pitch * PI / 180, staggered);
      }
    }
  }

  // reverse staggered balls because staggered batches are taken from the back
  if (staggered) {
    std::reverse(staggeredBalls.begin(), staggeredBalls.end());
  }
}

void AppLayer::addBall(float power, float yawOffset, float pitch,
                       bool staggered) {
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

  if (staggered) {
    staggeredBalls.push_back(finalDir);
  } else {
    addBall(finalDir);
  }
}

void AppLayer::addBall(glm::vec3 velocity) {
  glm::vec2 startPositionAbs = terrain.convertUV(startPosition);

  Ball ball(startPositionAbs.x,
            terrain.getHeightFromRelative(
                glm::vec2(startPosition.x * terrain.getWidth(),
                          startPosition.y * terrain.getHeight())) +
                terrain.getPosition().y + addBallRadius * 2,
            startPositionAbs.y, addBallRadius, addBallColor);
  ball.addPhysics(physicsWorld, physicsCommon, ballShapeRegistry);
  ball.setVelocity(velocity);

  balls.push_back(ball);
}

void AppLayer::writeOutputFile(std::ofstream& fout) {
  if (paramsNumDivisions * paramsNumDivisions * paramsNumDivisions !=
      balls.size()) {
    fout << "ERROR: "
         << paramsNumDivisions * paramsNumDivisions * paramsNumDivisions
         << " balls expected, " << balls.size() << " balls found." << std::endl;
    return;
  }

  fout << paramsNumDivisions << std::endl;
  fout << minPower << std::endl;
  fout << maxPower << std::endl;
  fout << minYaw << std::endl;
  fout << maxYaw << std::endl;
  fout << minPitch << std::endl;
  fout << maxPitch << std::endl;
  fout << balls[0].getRadius() << std::endl;
  fout << goal.getRadius() << std::endl;
  fout << std::endl;
  for (int i = 0; i < paramsNumDivisions; i++) {
    for (int j = 0; j < paramsNumDivisions; j++) {
      for (int k = 0; k < paramsNumDivisions; k++) {
        fout << balls[i * paramsNumDivisions * paramsNumDivisions +
                      j * paramsNumDivisions + k]
                    .getDistFromGoal(goal, terrain);
        if (k != paramsNumDivisions - 1) fout << " ";
      }
      fout << std::endl;
    }
    if (i != paramsNumDivisions - 1) fout << std::endl;
  }
}
