#include "engine/window.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

#include "engine/input/event_bus.h"
#include "shared/constants.h"

namespace engine {

window::window(input::event_bus &bus)
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

  glfwSetWindowUserPointer(base_window_, &bus);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): there is no way around this cast
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    throw std::runtime_error("Failed to initialize GLAD");
  }

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  glfwSetKeyCallback(base_window_, [](GLFWwindow *window, int key, int scancode,
                                      int action, int mode) {
    auto *bus_ptr =
        static_cast<input::event_bus *>(glfwGetWindowUserPointer(window));
    bus_ptr->emit(key, action);
  });

  // Disables vsync
  glfwSwapInterval(0);
}

window::~window() {
  glfwDestroyWindow(base_window_);
  glfwTerminate();
}

} // namespace engine
