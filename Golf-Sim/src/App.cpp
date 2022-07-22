#include "AppLayer.h"
#include "GLCore.h"
#include "GLFW/glfw3.h"
#include "implot.h"
#include <iostream>
#include <stb_image/stb_image.h>

using namespace GLCore;

class App : public Application {
 public:
  App()
      : Application("Golf Phase Space Simulator") {
    ImPlot::CreateContext();
    GetWindow().SetVSync(true);

    GLFWwindow* window =
        static_cast<GLFWwindow*>(GetWindow().GetNativeWindow());
    const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowSize(window, videoMode->width * 3 / 4, videoMode->width * 3 / 8);

    GLFWimage images[1];
    images[0].pixels = stbi_load("assets/icon/icon.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);
    
    PushLayer(new AppLayer(window));
  }

  ~App() { ImGui::DestroyContext(); }
};

int main() {
  std::unique_ptr<App> app = std::make_unique<App>();
  app->Run();
}
