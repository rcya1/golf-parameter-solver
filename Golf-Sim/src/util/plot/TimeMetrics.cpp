#include "TimeMetrics.h"

#include <imgui.h>
#include <implot.h>

#include <string>

void TimeMetrics::update(GLCore::Timestep& ts) {
  time += ts.GetSeconds();
  timePerFrameScrollBuffer.addPoint(time, ts.GetMilliseconds());
  timePerFrameRollBuffer.addPoint(time, ts.GetMilliseconds());
}

void TimeMetrics::imGuiRender(float dpiScale) {
  float sum = 0;
  for (int i = 0; i < timePerFrameRollBuffer.data.Size; i++) {
    sum += timePerFrameRollBuffer.data[i].y;
  }

  float average[] = {sum / timePerFrameRollBuffer.data.Size};

  if (ImGui::SliderFloat("History", &historyLength, 1, 30, "%.1f s")) {
    timePerFrameRollBuffer.span = historyLength;
  }

  static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
  ImPlot::SetNextPlotLimitsX(time - historyLength, time, ImGuiCond_Always);
  ImPlot::SetNextPlotLimitsY(0, 100.0);
  if (ImPlot::BeginPlot("##Scrolling", NULL, NULL,
                        ImVec2(-1, static_cast<int>(150 * dpiScale)), 0, flags,
                        flags)) {
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    ImPlot::PlotShaded("Time per Frame (ms)",
                       &timePerFrameScrollBuffer.data[0].x,
                       &timePerFrameScrollBuffer.data[0].y,
                       timePerFrameScrollBuffer.data.size(), -INFINITY,
                       timePerFrameScrollBuffer.offset, 2 * sizeof(float));
    ImPlot::PlotHLines("Average Time (ms)", average, 1);
    ImPlot::EndPlot();
  }
  ImPlot::SetNextPlotLimitsX(0, historyLength, ImGuiCond_Always);
  ImPlot::SetNextPlotLimitsY(0, 100.0);
  if (ImPlot::BeginPlot("##Rolling", NULL, NULL,
                        ImVec2(-1, static_cast<int>(150 * dpiScale)), 0, flags,
                        flags)) {
    ImPlot::PlotLine("Time per Frame (ms)", &timePerFrameRollBuffer.data[0].x,
                     &timePerFrameRollBuffer.data[0].y,
                     timePerFrameRollBuffer.data.size(), 0, 2 * sizeof(float));
    ImPlot::PlotHLines("Average Time (ms)", average, 1);
    ImPlot::EndPlot();
  }

  std::string averageText = "Average Time (ms): ";
  averageText += std::to_string(average[0]);
  ImGui::Text(averageText.c_str());
}
