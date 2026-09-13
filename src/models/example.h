#pragma once

#include <Eigen/Core>

namespace models {

struct example {
  Eigen::Vector2f square_position{0.0f, 0.0f};
  Eigen::Vector2f triangle_position{0.0f, -0.2f};
};

} // namespace models
