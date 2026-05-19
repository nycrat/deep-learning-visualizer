#include "application.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <print>
#include <stdexcept>

namespace {

const std::int64_t UI_SCALE{30};

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

    const int total_vertices{6};
    glDrawArraysInstanced(GL_TRIANGLES, 0, total_vertices,
                          shared::TOTAL_PIXELS);

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

  window_ = glfwCreateWindow(shared::GRID_SIZE * UI_SCALE,
                             shared::GRID_SIZE * UI_SCALE, "digit classifier",
                             nullptr, nullptr);
  if (!window_) {
    throw std::runtime_error("Failed to create GLFW window");
  }
  glfwMakeContextCurrent(window_);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): there is no way around this cast
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    throw std::runtime_error("Failed to initialize GLAD");
  }

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  glfwSetKeyCallback(window_, [](GLFWwindow *window, int key, int scancode,
                                 int action, int mods) {
    auto *app = static_cast<application *>(glfwGetWindowUserPointer(window));

    if (app) {
      app->handle_key(key, action);
    }
  });
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
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

  constexpr float square_length{2.0f / shared::GRID_SIZE};

  // clang-format off
  // Square in top left corner, side length of square_length
  constexpr auto vertices = std::to_array({
    // x     y
    -1.0f, 1.0f,
    -1.0f + square_length, 1.0f,
    -1.0f, 1.0f - square_length,

    -1.0f, 1.0f - square_length,
    -1.0f + square_length, 1.0f,
    -1.0f + square_length, 1.0f - square_length,
  });
  // clang-format on

  std::uint32_t vertex_vbo{};
  glGenBuffers(1, &vertex_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  glGenBuffers(1, &instance_vbo_);
  buffer_image_data();
  glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr);
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 1);

  square_program_ = std::make_unique<program>("src/shaders/square.vert",
                                              "src/shaders/square.frag");
  square_program_->use();
}

void application::buffer_image_data() {
  glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(drawn_image_.size() * sizeof(float)),
               drawn_image_.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void application::handle_key(int key, int action) {
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_SPACE) {
      Eigen::Map<Eigen::VectorXf> input{drawn_image_.data(),
                                        shared::GRID_SIZE * shared::GRID_SIZE};

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

  const double cursor_offset{0.5};
  const double brush_size{1.75};

  double scaled_x = x / UI_SCALE - cursor_offset;
  double scaled_y = y / UI_SCALE - cursor_offset;

  for (int i{0}; i < shared::GRID_SIZE; i++) {
    for (int j{0}; j < shared::GRID_SIZE; j++) {
      double distance_x{i - scaled_x};
      double distance_y{j - scaled_y};
      double distance{sqrt(distance_x * distance_x + distance_y * distance_y)};
      auto pixel_index = i + j * shared::GRID_SIZE;

      drawn_image_.at(pixel_index) = std::max(
          static_cast<float>(-std::pow(distance / brush_size, 3) + 1.0),
          drawn_image_.at(pixel_index));
    }
  }

  buffer_image_data();
}

} // namespace ui
