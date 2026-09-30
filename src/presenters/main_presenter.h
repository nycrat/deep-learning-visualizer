#pragma once

/** @file */

#include "engine/input/constants.h"
#include "engine/input/event_bus.h"
#include "models/mnist.h"
#include "shared/constants.h"
#include "views/network_view.h"
#include "views/pixel_grid_view.h"
#include <array>
#include <functional>
#include <memory>

namespace presenters {

/**
 * A presenter for the interactive MNIST pixel grid prediction.
 *
 * Holds the image being drawn and keeps the network's activations in step with
 * it, so the network diagram reflects every stroke rather than only the last
 * prediction.
 */
class main_presenter {
public:
  main_presenter(std::unique_ptr<models::mnist> model,
                 std::unique_ptr<views::pixel_grid_view> view,
                 std::unique_ptr<views::network_view> network_view,
                 engine::input::event_bus &bus);

  void render();

  void handle_key_event(engine::input::key key, engine::input::action action);

  void handle_mouse_event(engine::input::mouse mouse,
                          engine::input::action action);

  void handle_cursor_event(const Eigen::Vector2f &cursor_position);

private:
  void draw_image();

  /**
   * Runs a forward pass over the drawn image, so that the network view has
   * activations to draw.
   *
   * @return The predicted value.
   */
  int propagate();

  std::unique_ptr<models::mnist> model_;
  std::unique_ptr<views::pixel_grid_view> drawing_view_;
  std::unique_ptr<views::network_view> network_view_;
  std::reference_wrapper<engine::input::event_bus> bus_;

  std::array<float, shared::TOTAL_PIXELS> drawn_image_{};
  bool mouse_pressed_{false};
};

} // namespace presenters
