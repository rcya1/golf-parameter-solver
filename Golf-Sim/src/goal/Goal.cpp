#include "Goal.h"

#include <goal/GoalRenderer.h>

Goal::Goal(float x, float z, float r)
    : relativePosition(x, z), radius(r), color(0.1f, 0.35f, 0.1f) {}

void Goal::generateModel(Terrain& terrain) {
  freeModel();

  glm::vec2 relativeCoords =
      glm::vec2(relativePosition.x * (terrain.getWidth() - 2 * radius) + radius,
                relativePosition.y * (terrain.getHeight() - 2 * radius) + radius);

  goalModel.generateModel(terrain, relativeCoords, radius);
}

void Goal::freeModel() { goalModel.freeModel(); }

void Goal::render(GoalRenderer& renderer) {
  renderer.add(GoalRenderJob{goalModel, color});
}

void Goal::imGuiRender(reactphysics3d::PhysicsWorld* physicsWorld,
                       reactphysics3d::PhysicsCommon& physicsCommon,
                       Terrain& terrain) {
  ImGui::Begin("Goal Controls");
  ImGui::DragFloat2("Position", glm::value_ptr(relativePosition), 0.01f, 0.0f, 1.0f);
  ImGui::DragFloat("Radius", &radius, 0.1f, 0.1f, 5.0f);
  ImGui::ColorEdit3("Color", glm::value_ptr(color));

  if (ImGui::Button("Regenerate Goal")) {
    generateModel(terrain);
  }
  ImGui::End();
}

void Goal::addPhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                      reactphysics3d::PhysicsCommon& physicsCommon) {}

void Goal::removePhysics(reactphysics3d::PhysicsWorld* physicsWorld,
                         reactphysics3d::PhysicsCommon& physicsCommon) {}

glm::vec2 Goal::getAbsolutePosition(Terrain& terrain) { 
  return glm::vec2(terrain.getPosition().x - terrain.getWidth() / 2 + radius + relativePosition.x * (terrain.getWidth() - 2 * radius),
    terrain.getPosition().z - terrain.getHeight() / 2 + radius + relativePosition.y * (terrain.getHeight() - 2 * radius));
}
