#pragma once

/// @file

#include <print>

#include "views/example_view.h"

namespace views {

class example_view_print : public example_view {
public:
  void display_square(const Eigen::Vector2f &position) override {
    std::println("rendering square at ({},{})", position.x(), position.y());
  }

  void display_triangle(const Eigen::Vector2f &position) override {
    std::println("rendering triangle at ({},{})", position.x(), position.y());
  }

  void display_line(const Eigen::Vector2f &p1,
                    const Eigen::Vector2f &p2) override {
    std::println("rendering line from ({},{}) to ({},{})", p1.x(), p1.y(),
                 p2.x(), p2.y());
  }
};

} // namespace views
