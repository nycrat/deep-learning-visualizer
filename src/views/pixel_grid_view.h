#pragma once

/** @file */

#include "engine/graphics/renderer.h"
#include "shared/constants.h"
#include <Eigen/Core>
#include <array>
#include <functional>

namespace views {

class pixel_grid_view {
public:
  explicit pixel_grid_view(engine::graphics::renderer &renderer)
      : renderer_(renderer) {
  }

  void draw_grid(const std::array<float, shared::TOTAL_PIXELS> &image_data) {
    for (int x{0}; x < shared::GRID_SIZE; x++) {
      for (int y{0}; y < shared::GRID_SIZE; y++) {
        const auto value{image_data.at(x + y * shared::GRID_SIZE)};
        draw_square(x, y, value);
      }
    }
  }

private:
  void draw_square(int x, int y, float value) {
    const Eigen::Vector2f anchor{
        static_cast<float>(x) * 2 / shared::GRID_SIZE - 1.0f,
        static_cast<float>(y) * 2 / shared::GRID_SIZE - 1.0f};
    const float size{2.0f / shared::GRID_SIZE};

    renderer_.get().set_color(Eigen::Vector3f{1.0f, 1.0f, 1.0f} * value);
    renderer_.get().draw_quad(anchor + Eigen::Vector2f{0, 0},
                              anchor + Eigen::Vector2f{0, size},
                              anchor + Eigen::Vector2f{size, size},
                              anchor + Eigen::Vector2f{size, 0});
  }

  std::reference_wrapper<engine::graphics::renderer> renderer_;
};

} // namespace views
