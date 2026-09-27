#include "presenters/pixel_grid_presenter.h"

#include <print>
#include <utility>

namespace presenters {

pixel_grid_presenter::pixel_grid_presenter(
    std::unique_ptr<models::mnist> model,
    std::unique_ptr<views::pixel_grid_view> view, engine::input::event_bus &bus)
    : model_(std::move(model)), view_(std::move(view)), bus_(bus) {
  using namespace engine::input;
  bus.subscribe([&](key k, auto a) { this->handle_key_event(k, a); });
  bus.subscribe([&](mouse m, auto a) { this->handle_mouse_event(m, a); });
  bus.subscribe([&](auto pos) { this->handle_cursor_event(pos); });
}

void pixel_grid_presenter::render() {
  view_->draw_grid(drawn_image_);
}

void pixel_grid_presenter::handle_key_event(engine::input::key key,
                                            engine::input::action action) {
  using namespace engine::input;
  if (action == action::down) {
    if (key == key::space) {
      Eigen::Map<Eigen::VectorXf> input{drawn_image_.data(),
                                        shared::GRID_SIZE * shared::GRID_SIZE};

      std::println("{}", model_->predict(input));
    } else if (key == key::r) {
      drawn_image_.fill(0.0f);
    } else if (key == key::t) {
      model_->train();
    } else if (key == key::y) {
      model_->test();
    } else if (key == key::s) {
      model_->to_file("data/mnist/trained.mlp");
    }
  }
}

void pixel_grid_presenter::handle_mouse_event(engine::input::mouse mouse,
                                              engine::input::action action) {
  if (mouse == engine::input::mouse::button_left) {
    mouse_pressed_ = action == engine::input::action::down;
  }
}

void pixel_grid_presenter::handle_cursor_event(
    const Eigen::Vector2f &cursor_position) {
  if (!mouse_pressed_) {
    return;
  }

  const double cursor_offset{0.5};
  const double brush_size{1.75};

  double scaled_x =
      (cursor_position.x() + 1.0f) / 2 * shared::GRID_SIZE - cursor_offset;
  double scaled_y =
      (cursor_position.y() + 1.0f) / 2 * shared::GRID_SIZE - cursor_offset;

  for (int i{0}; i < shared::GRID_SIZE; i++) {
    for (int j{0}; j < shared::GRID_SIZE; j++) {
      double distance_x{i - scaled_x};
      double distance_y{j - scaled_y};
      double distance{sqrt(distance_x * distance_x + distance_y * distance_y)};
      auto pixel_index = i + j * shared::GRID_SIZE;

      drawn_image_.at(pixel_index) = std::max(
          static_cast<float>(-std::pow(distance / brush_size, 3) + 1.0),
          drawn_image_.at(pixel_index));
    }
  }
}

} // namespace presenters
