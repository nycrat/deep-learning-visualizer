#pragma once

#include "engine/graphics/shader.h"

#include <Eigen/Core>
#include <vector>

// #include <freetype2/ft2build.h>
// #include FT_FREETYPE_H

namespace engine::graphics {

class renderer {
public:
  renderer();
  ~renderer();

  void begin_frame();
  void end_frame();

  void draw_triangle(const Eigen::Vector2f &p1, const Eigen::Vector2f &p2,
                     const Eigen::Vector2f &p3);
  void draw_quad(const Eigen::Vector2f &p1, const Eigen::Vector2f &p2,
                 const Eigen::Vector2f &p3, const Eigen::Vector2f &p4);
  void draw_line(const Eigen::Vector2f &p1, const Eigen::Vector2f &p2,
                 float thickness);
  void draw_text(const Eigen::Vector2f &position, const std::string &text);

private:
  shader square_program_{"src/engine/graphics/shaders/example.vert",
                         "src/engine/graphics/shaders/example.frag"};
  std::vector<Eigen::Vector2f> vertices_{};
  unsigned int vao_{};
  unsigned int vbo_{};

  static constexpr auto MAX_VERTICES{100000};
};

} // namespace engine::graphics
