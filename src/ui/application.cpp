#include "application.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <print>
#include <stdexcept>

namespace {

const int GRID_SIZE{28};

} // namespace

namespace ui {

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
  glfwWindowHint(GLFW_RESIZABLE, 0);

  window_ = glfwCreateWindow(GRID_SIZE * 30, GRID_SIZE * 30, "digit classifier",
                             nullptr, nullptr);
  if (!window_) {
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwMakeContextCurrent(window_);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw std::runtime_error("Failed to initialize GLAD");
  }

  glfwSetKeyCallback(window_, [](GLFWwindow *window, int key, int scancode,
                                 int action, int mods) {
    auto *app = static_cast<application *>(glfwGetWindowUserPointer(window));

    if (app) {
      app->handle_key(key, action);
    }
  });
  glfwSetMouseButtonCallback(window_, [](GLFWwindow *window, int button,
                                         int action, int mods) {
    auto *app = static_cast<application *>(glfwGetWindowUserPointer(window));

    if (app) {
      app->handle_mouse(button, action);
    }
  });
  glfwSetCursorPosCallback(window_, [](GLFWwindow *window, double x, double y) {
    auto *app = static_cast<application *>(glfwGetWindowUserPointer(window));

    if (app) {
      app->handle_cursor_pos(x, y);
    }
  });

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
  glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);
  glBufferData(GL_ARRAY_BUFFER, drawn_image_.size() * sizeof(float),
               drawn_image_.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void application::handle_key(int key, int action) {
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_SPACE) {
      Eigen::Map<Eigen::VectorXf> input{drawn_image_.data(),
                                        GRID_SIZE * GRID_SIZE};

      std::println("{}", model_.predict(input));
    } else if (key == GLFW_KEY_R) {
      drawn_image_.fill(0.0f);
      buffer_image_data();
    }
  }
}

void application::handle_mouse(int button, int action) {
  mouse_pressed_ = action == GLFW_PRESS;
  handle_cursor_pos(cursor_x_, cursor_y_);
}

void application::handle_cursor_pos(double x, double y) {
  cursor_x_ = x;
  cursor_y_ = y;

  if (!mouse_pressed_) {
    return;
  }

  auto scaled_x = x / 30.0 - 0.5;
  auto scaled_y = y / 30.0 - 0.5;

  double brush_size{1.75};

  for (int i{0}; i < GRID_SIZE; i++) {
    for (int j{0}; j < GRID_SIZE; j++) {
      double distance_x{i - scaled_x};
      double distance_y{j - scaled_y};
      double distance{sqrt(distance_x * distance_x + distance_y * distance_y)};
      drawn_image_[i + j * GRID_SIZE] = std::max(
          static_cast<float>(-std::pow(distance / brush_size, 3) + 1.0),
          drawn_image_[i + j * GRID_SIZE]);
    }
  }

  buffer_image_data();
}

} // namespace ui
