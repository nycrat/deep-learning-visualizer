#pragma once

/** @file */

#include "engine/input/constants.h"
#include "engine/input/event_bus.h"
#include "models/mnist.h"
#include "shared/constants.h"
#include "views/pixel_grid_view.h"
#include <array>
#include <functional>
#include <memory>

namespace presenters {

/**
 * A presenter for the interactive MNIST pixel grid prediction.
 */
class pixel_grid_presenter {
public:
  pixel_grid_presenter(std::unique_ptr<models::mnist> model,
                       std::unique_ptr<views::pixel_grid_view> view,
                       engine::input::event_bus &bus);

  void render();

  void handle_key_event(engine::input::key key, engine::input::action action);

  void handle_mouse_event(engine::input::mouse mouse,
                          engine::input::action action);

  void handle_cursor_event(const Eigen::Vector2f &cursor_position);

private:
  std::unique_ptr<models::mnist> model_;
  std::unique_ptr<views::pixel_grid_view> view_;
  std::reference_wrapper<engine::input::event_bus> bus_;

  std::array<float, shared::TOTAL_PIXELS> drawn_image_{};
  bool mouse_pressed_{false};
};

} // namespace presenters
