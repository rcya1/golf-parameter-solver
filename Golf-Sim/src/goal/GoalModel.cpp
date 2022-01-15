#include "GoalModel.h"

GoalModel::GoalModel() {}

void GoalModel::generateModel() {
  freeModel();
}

void GoalModel::freeModel() {
  if (vertices.empty()) return;

  vertices.clear();

  vertexArray->free();
  vertexBuffer->free();
}
