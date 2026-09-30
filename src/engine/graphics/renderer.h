#pragma once

/** @file */

#include "engine/graphics/shader.h"

#include <Eigen/Core>
#include <vector>

// #include <freetype2/ft2build.h>
// #include FT_FREETYPE_H

namespace engine::graphics {

/**
 * OpenGL renderer that uses batch rendering.
 */
class renderer {
public:
  /**
   * Constructs the renderer; reserves memory in the vertices vector.
   */
  renderer();

  /**
   * Destructor that cleans up the VAO and VBO.
   */
  ~renderer();

  renderer(const renderer &) = delete;
  renderer &operator=(const renderer &) = delete;
  renderer(renderer &&) = delete;
  renderer &operator=(renderer &&) = delete;

  /**
   * Begins a frame by clearing the vertices buffer.
   *
   * Should be called before submitting shapes to the renderer.
   */
  void begin_frame();

  /**
   * Ends a frame by clearing what is currently drawn and drawing the vertices.
   */
  void end_frame();

  /**
   * Sets the current color used when buffering shapes to be drawn.
   *
   * @param color A RGB color vector.
   */
  void set_color(const Eigen::Vector3f &color) {
    color_ = color;
  }

  /**
   * Checks if the frame has no room left for another shape.
   */
  [[nodiscard]] bool is_full() const {
    return vertices_.size() + VERTICES_PER_QUAD > MAX_VERTICES;
  }

  /**
   * Buffers a triangle to be drawn when the frame ends.
   *
   * @param p1 The first point of the triangle.
   * @param p2 The second point of the triangle.
   * @param p3 The third point of the triangle.
   */
  void draw_triangle(const Eigen::Vector2f &p1, const Eigen::Vector2f &p2,
                     const Eigen::Vector2f &p3);

  /**
   * Buffers a quad to be drawn when the frame ends.
   *
   * @param p1 The first point of the quad.
   * @param p2 The second point of the quad.
   * @param p3 The third point of the quad.
   * @param p4 The fourth point of the quad.
   */
  void draw_quad(const Eigen::Vector2f &p1, const Eigen::Vector2f &p2,
                 const Eigen::Vector2f &p3, const Eigen::Vector2f &p4);

  /**
   * Buffers a line to be drawn when the frame ends.
   *
   * @param p1 The first point of the line.
   * @param p2 The second point of the line.
   * @param thickness The thickness of the line.
   */
  void draw_line(const Eigen::Vector2f &p1, const Eigen::Vector2f &p2,
                 float thickness);

  /**
   * Buffers text to be drawn when the frame ends.
   *
   * TODO: function is not implemented yet
   */
  void draw_text(const Eigen::Vector2f &position, const std::string &text);

private:
  struct vertex {
    Eigen::Vector2f position;
    Eigen::Vector3f color;
  };

  shader program_{"src/engine/graphics/shaders/generic.vert",
                  "src/engine/graphics/shaders/generic.frag"};
  std::vector<vertex> vertices_{};
  Eigen::Vector3f color_{};
  unsigned int vao_{};
  unsigned int vbo_{};

  static constexpr auto MAX_VERTICES{100000uz};
  static constexpr auto VERTICES_PER_QUAD{6uz};
};

} // namespace engine::graphics
