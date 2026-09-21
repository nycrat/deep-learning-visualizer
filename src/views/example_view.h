#pragma once

#include <Eigen/Core>

namespace views {

class example_view {
public:
  virtual ~example_view() = default;

  virtual void display_square(const Eigen::Vector2f &position) = 0;
  virtual void display_triangle(const Eigen::Vector2f &position) = 0;
  virtual void display_line(const Eigen::Vector2f &position) = 0;
};

} // namespace views
