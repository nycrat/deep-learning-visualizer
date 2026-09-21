#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <functional>
#include <numbers>

#include "engine/graphics/renderer.h"
#include "views/example_view.h"

namespace views {

class example_view_gl : public example_view {
public:
  explicit example_view_gl(engine::graphics::renderer &renderer)
      : renderer_(renderer) {
  }

  void display_square(const Eigen::Vector2f &position) override {
    const auto offset{0.1f / std::numbers::sqrt2_v<float>};
    const auto top_left = position + Eigen::Vector2f{-offset, offset};
    const auto top_right = position + Eigen::Vector2f{offset, offset};
    const auto bottom_right = position + Eigen::Vector2f{offset, -offset};
    const auto bottom_left = position + Eigen::Vector2f{-offset, -offset};
    renderer_.get().draw_quad(top_left, top_right, bottom_right, bottom_left);
  }

  void display_triangle(const Eigen::Vector2f &position) override {
    const Eigen::Rotation2D<float> rotation{std::numbers::pi_v<float> * 2.0f /
                                            3.0f};
    const auto offset = Eigen::Vector2f{0.0, 0.1};
    const auto p1 = position + offset;
    const auto p2 = position + rotation * offset;
    const auto p3 = position + rotation * (rotation * offset);
    renderer_.get().draw_triangle(p1, p2, p3);
  }

  void display_line(const Eigen::Vector2f &p1,
                    const Eigen::Vector2f &p2) override {
    const auto thickness{0.02f};
    renderer_.get().draw_line(p1, p2, thickness);
  }

private:
  std::reference_wrapper<engine::graphics::renderer> renderer_;
};

} // namespace views
