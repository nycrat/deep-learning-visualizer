#pragma once

/** @file */

#include "engine/graphics/renderer.h"
#include "shared/layout.h"
#include "views/pixel_grid_view.h"
#include <functional>

namespace views {

class pixel_grid_view_gl : public pixel_grid_view {
public:
  explicit pixel_grid_view_gl(engine::graphics::renderer &renderer)
      : renderer_(renderer) {
  }

  void draw_grid(
      const std::array<float, shared::TOTAL_PIXELS> &image_data) override {
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
        shared::GRID_LEFT + (static_cast<float>(x) / shared::GRID_SIZE) *
                                (shared::GRID_RIGHT - shared::GRID_LEFT),
        shared::GRID_BOTTOM + (static_cast<float>(y) / shared::GRID_SIZE) *
                                  (shared::GRID_TOP - shared::GRID_BOTTOM)};

    const float width{(shared::GRID_RIGHT - shared::GRID_LEFT) /
                      shared::GRID_SIZE};
    const float height{(shared::GRID_TOP - shared::GRID_BOTTOM) /
                       shared::GRID_SIZE};

    renderer_.get().set_color(Eigen::Vector4f{1.0f, 1.0f, 1.0f, value});
    renderer_.get().draw_quad(anchor + Eigen::Vector2f{0, 0},
                              anchor + Eigen::Vector2f{0, height},
                              anchor + Eigen::Vector2f{width, height},
                              anchor + Eigen::Vector2f{width, 0});
  }

  std::reference_wrapper<engine::graphics::renderer> renderer_;
};

} // namespace views
