#include "application.h"

#include <array>
#include <print>
#include <ranges>
#include <span>
#include <stdexcept>
#include <vector>

namespace {

const int GRID_SIZE{28};

} // namespace

namespace avah {

application::application() {
  initialize_glfw();
  initialize_opengl();
}

application::~application() {
  square_program_.reset();
  glfwDestroyWindow(window_);
  glfwTerminate();
}

void application::run() {
  while (!glfwWindowShouldClose(window_)) {
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(square_vao_);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GRID_SIZE * GRID_SIZE);

    glfwSwapBuffers(window_);

    glfwWaitEvents();
  }
}

void application::initialize_glfw() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window_ = glfwCreateWindow(800, 800, "digit classifier", nullptr, nullptr);
  if (!window_) {
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwMakeContextCurrent(window_);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw std::runtime_error("Failed to initialize GLAD");
  }

  glfwSetKeyCallback(window_, application::key_callback);

  // Disables vsync
  glfwSwapInterval(0);

  // Store application object inside GLFW window
  glfwSetWindowUserPointer(window_, this);
}

void application::initialize_opengl() {
  glGenVertexArrays(1, &square_vao_);
  glBindVertexArray(square_vao_);

  float square_length{2.0f / static_cast<float>(GRID_SIZE)};

  // clang-format off
  // Square in top left corner, side length of square_length
  std::array<float, 6 * 2> vertices{
    // x     y
    -1.0f, 1.0f,
    -1.0f + square_length, 1.0f,
    -1.0f, 1.0f - square_length,

    -1.0f, 1.0f - square_length,
    -1.0f + square_length, 1.0f,
    -1.0f + square_length, 1.0f - square_length,
  };
  // clang-format on

  std::uint32_t vertex_vbo{};
  glGenBuffers(1, &vertex_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(0);

  glGenBuffers(1, &instance_vbo_);
  buffer_image_data();
  glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 1);

  square_program_ = std::make_unique<program>("src/shaders/square.vert",
                                              "src/shaders/square.frag");
  square_program_->use();
}

void application::buffer_image_data() {
  auto image =
      training_images_.data().subspan(GRID_SIZE * GRID_SIZE * digit_index_,
                                      GRID_SIZE * GRID_SIZE) |
      std::views::transform(
          [](auto x) { return static_cast<float>(x) / 255.0f; }) |
      std::ranges::to<std::vector<float>>();

  glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);
  glBufferData(GL_ARRAY_BUFFER, image.size() * sizeof(float), image.data(),
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void application::handle_key(int key, int action) {
  if (action == GLFW_PRESS) {
    bool changed{false};

    if (key == GLFW_KEY_LEFT) {
      digit_index_ =
          digit_index_ == 0 ? training_images_.rows() - 1 : digit_index_ - 1;
      changed = true;
    }
    if (key == GLFW_KEY_RIGHT) {
      digit_index_ =
          digit_index_ == training_images_.rows() - 1 ? 0 : digit_index_ + 1;
      changed = true;
    }
    if (changed) {
      buffer_image_data();
      std::println("Digit at index {}: {}", digit_index_,
                   training_labels_.data()[digit_index_]);
    }
  }
}

void application::key_callback(GLFWwindow *window, int key, int scancode,
                               int action, int mods) {
  auto *app = static_cast<application *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->handle_key(key, action);
  }
}

} // namespace avah
