#include "AppLayer.h"
#include "GLCore.h"
#include "GLFW/glfw3.h"
#include "implot.h"

using namespace GLCore;

class App : public Application {
 public:
  App()
      : Application("Golf Phase Space Simulator") {
    ImPlot::CreateContext();

    GLFWwindow* window =
        static_cast<GLFWwindow*>(GetWindow().GetNativeWindow());
    const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor()); 
    glfwSetWindowSize(window, videoMode->width * 3 / 4, videoMode->width * 3 / 8);
    
    PushLayer(new AppLayer(window));
  }

  ~App() { ImGui::DestroyContext(); }
};

int main() {
  std::unique_ptr<App> app = std::make_unique<App>();
  app->Run();
}
