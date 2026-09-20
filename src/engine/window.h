#pragma once

#include "engine/input/event_bus.h"
#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace engine {

class window {
public:
  window(input::event_bus &bus);
  ~window();

  window(const window &) = delete;
  window(window &&) = delete;
  window &operator=(const window &) = delete;
  window &operator=(window &&) = delete;

  [[nodiscard]] bool should_close() const {
    return glfwWindowShouldClose(base_window_);
  }

  void wait_events() {
    glfwWaitEvents();
  }

  void swap_buffers() {
    glfwSwapBuffers(base_window_);
  }

private:
  GLFWwindow *base_window_{nullptr};

  static constexpr int UI_SCALE{30};
};

} // namespace engine
