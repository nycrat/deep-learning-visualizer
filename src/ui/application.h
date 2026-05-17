#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "mlp/mnist.h"
#include "program.h"

namespace ui {

class application {
public:
  application();
  ~application();
  application(const application &) = delete;
  application &operator=(const application &) = delete;
  application(application &&) = delete;
  application &operator=(application &&) = delete;

  void run();

private:
  void initialize_glfw();
  void initialize_opengl();
  void buffer_image_data();
  void handle_key(int key, int action);
  void handle_mouse(int button, int action);
  void handle_cursor_pos(double x, double y);

  GLFWwindow *window_{nullptr};

  std::uint32_t square_vao_{};
  std::uint32_t instance_vbo_{};
  std::unique_ptr<program> square_program_{nullptr};

  std::array<float, 28 * 28> drawn_image_{};

  mlp::mnist model_;

  bool mouse_pressed_{};
  double cursor_x_{};
  double cursor_y_{};
};

} // namespace ui
