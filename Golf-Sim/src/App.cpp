#include "AppLayer.h"
#include "GLCore.h"
#include "GLFW/glfw3.h"

using namespace GLCore;

class App : public Application {
 public:
  App() : Application("Golf Sim", 3200, 1800) {
    // Disable cursor
    glfwSetInputMode(static_cast<GLFWwindow*>(GetWindow().GetNativeWindow()),
                     GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    PushLayer(new AppLayer());
  }
};

int main() {
  std::unique_ptr<App> app = std::make_unique<App>();
  app->Run();
}
