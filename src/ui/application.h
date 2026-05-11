#pragma once

#include <cstdint>
#include <memory>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "program.h"
#include "shared/idx_matrix.h"

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
  static void key_callback(GLFWwindow *window, int key, int scancode,
                           int action, int mods);

  GLFWwindow *window_{nullptr};

  std::uint32_t square_vao_{};
  std::uint32_t instance_vbo_{};
  std::unique_ptr<program> square_program_{nullptr};

  dl::idx_matrix training_labels_{"data/mnist/train-labels.idx"};
  dl::idx_matrix training_images_{"data/mnist/train-images.idx"};

  std::size_t digit_index_{};
};

} // namespace ui
