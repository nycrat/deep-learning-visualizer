#include "engine/graphics/renderer.h"

#include <glad/glad.h>
#include <stdexcept>

namespace engine::graphics {

renderer::renderer() {
  vertices_.reserve(MAX_VERTICES);

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glGenBuffers(1, &vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);

  // Pre-allocate memory block on GPU
  glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Eigen::Vector2f), nullptr,
               GL_DYNAMIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector2f),
                        nullptr);
  glEnableVertexAttribArray(0);
}

renderer::~renderer() {
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
}

void renderer::begin_frame() {
}

void renderer::end_frame() {
  glClearColor(1.0, 1.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  square_program_.use();
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(
      GL_ARRAY_BUFFER, 0,
      static_cast<GLsizeiptr>(vertices_.size() * sizeof(Eigen::Vector2f)),
      vertices_.data());

  glBindVertexArray(vao_);
  glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()));

  vertices_.clear();
}

void renderer::draw_triangle(const Eigen::Vector2f &p1,
                             const Eigen::Vector2f &p2,
                             const Eigen::Vector2f &p3) {
  vertices_.push_back(p1);
  vertices_.push_back(p2);
  vertices_.push_back(p3);
}

void renderer::draw_quad(const Eigen::Vector2f &p1, const Eigen::Vector2f &p2,
                         const Eigen::Vector2f &p3, const Eigen::Vector2f &p4) {
  draw_triangle(p1, p2, p3);
  draw_triangle(p3, p4, p1);
}

void renderer::draw_line(const Eigen::Vector2f &p1, const Eigen::Vector2f &p2,
                         float thickness) {
  const auto parallel{(p1 - p2).normalized() * thickness / 2.0f};
  const Eigen::Vector2f perp{parallel.y(), -parallel.x()};
  draw_quad(p1 - perp, p1 + perp, p2 + perp, p2 - perp);
}

void renderer::draw_text(const Eigen::Vector2f &position,
                         const std::string &text) {
  // TODO implement this function
  throw std::runtime_error("renderer::draw_text is not implemented yet");
}

} // namespace engine::graphics
