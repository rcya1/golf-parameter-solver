#include "AppLayer.h"
#include "GLCore.h"
#include "GLFW/glfw3.h"
#include "implot.h"

using namespace GLCore;

class App : public Application {
 public:
  App() : Application("Golf Phase Space Simulator", 3200, 1800) {
    ImPlot::CreateContext();
    PushLayer(
        new AppLayer(static_cast<GLFWwindow*>(GetWindow().GetNativeWindow())));
  }

  ~App() { ImGui::DestroyContext(); }
};

int main() {
  std::unique_ptr<App> app = std::make_unique<App>();
  app->Run();
}
