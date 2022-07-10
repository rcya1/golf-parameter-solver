#pragma once

#include "GLCore.h"

inline void setupGreenButton() {
  ImGui::PushStyleColor(ImGuiCol_Button,
                        ImVec4(0 / 255.0f, 162 / 255.0f, 62 / 255.0f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        ImVec4(0 / 255.0f, 130 / 255.0f, 50 / 255.0f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        ImVec4(0 / 255.0f, 104 / 255.0f, 40 / 255.0f, 1.0f));
}

inline void setupRedButton() {
  ImGui::PushStyleColor(ImGuiCol_Button,
                        ImVec4(222 / 255.0f, 5 / 255.0f, 0 / 255.0f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        ImVec4(178 / 255.0f, 4 / 255.0f, 0 / 255.0f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        ImVec4(142 / 255.0f, 3 / 255.0f, 0 / 255.0f, 1.0f));
}

inline void clearButtonStyle() {
  ImGui::PopStyleColor(3);
}