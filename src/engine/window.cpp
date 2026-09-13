#include "engine/window.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

#include "shared/constants.h"

namespace engine {

window::window()
    : base_window_([] {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, 0);

        return glfwCreateWindow(shared::GRID_SIZE * UI_SCALE,
                                shared::GRID_SIZE * UI_SCALE,
                                "digit classifier", nullptr, nullptr);
      }()) {
  if (!base_window_) {
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwMakeContextCurrent(base_window_);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): there is no way around this cast
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    throw std::runtime_error("Failed to initialize GLAD");
  }

  // Disables vsync
  glfwSwapInterval(0);
}

window::~window() {
  glfwDestroyWindow(base_window_);
  glfwTerminate();
}

} // namespace engine
