#pragma once

/** @file */

#include "shared/constants.h"
#include <Eigen/Core>
#include <array>

namespace views {

class pixel_grid_view {
public:
  virtual ~pixel_grid_view() = default;
  virtual void
  draw_grid(const std::array<float, shared::TOTAL_PIXELS> &image_data) = 0;
};

} // namespace views
