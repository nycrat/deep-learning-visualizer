#pragma once

/** @file */

#include "engine/input/event_bus.h"
#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace engine {

/**
 * A GLFW window manager.
 */
class window {
public:
  /**
   * Constructs a new window manager.
   *
   * @param bus The input event bus to attach GLFW window event callbacks to.
   */
  explicit window(input::event_bus &bus);

  /**
   * Destructor that cleans up GLFW resources.
   */
  ~window();

  window(const window &) = delete;
  window(window &&) = delete;
  window &operator=(const window &) = delete;
  window &operator=(window &&) = delete;

  /**
   * Returns true if the GLFW window should be closed.
   */
  [[nodiscard]] bool should_close() const {
    return glfwWindowShouldClose(base_window_);
  }

  /**
   * Blocks the current theread until there is a new window event.
   */
  void wait_events() {
    glfwWaitEvents();
  }

  /**
   * Switches to the newly rendered frame.
   */
  void swap_buffers() {
    glfwSwapBuffers(base_window_);
  }

private:
  GLFWwindow *base_window_{nullptr};

  static constexpr int UI_SCALE{30};
};

} // namespace engine
