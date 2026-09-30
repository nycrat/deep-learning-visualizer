#include "presenters/main_presenter.h"

#include "shared/layout.h"

#include <print>
#include <utility>

namespace presenters {

main_presenter::main_presenter(
    std::unique_ptr<models::mnist> model,
    std::unique_ptr<views::pixel_grid_view> view,
    std::unique_ptr<views::network_view> network_view,
    engine::input::event_bus &bus)
    : model_(std::move(model)), drawing_view_(std::move(view)),
      network_view_(std::move(network_view)), bus_(bus) {
  using namespace engine::input;
  bus.subscribe([&](key k, auto a) { this->handle_key_event(k, a); });
  bus.subscribe([&](mouse m, auto a) { this->handle_mouse_event(m, a); });
  bus.subscribe([&](auto pos) { this->handle_cursor_event(pos); });
}

void main_presenter::render() {
  draw_image();
  network_view_->draw_network(model_->layers());
}

void main_presenter::handle_key_event(engine::input::key key,
                                      engine::input::action action) {
  using namespace engine::input;
  if (action == action::down) {
    if (key == key::space) {
      Eigen::Map<Eigen::VectorXf> input{drawn_image_.data(),
                                        shared::GRID_SIZE * shared::GRID_SIZE};

      std::println("{}", model_->predict(input));
    } else if (key == key::r) {
      drawn_image_.fill(0.0f);
      propagate();
    } else if (key == key::t) {
      model_->train();
    } else if (key == key::y) {
      model_->test();
    } else if (key == key::s) {
      model_->to_file("data/mnist/trained.mlp");
    }
  }
}

void main_presenter::handle_mouse_event(engine::input::mouse mouse,
                                        engine::input::action action) {
  if (mouse == engine::input::mouse::button_left) {
    mouse_pressed_ = action == engine::input::action::down;
  }
}

void main_presenter::handle_cursor_event(
    const Eigen::Vector2f &cursor_position) {
  if (!mouse_pressed_ || !shared::is_over_grid(cursor_position.x())) {
    return;
  }

  const double cursor_offset{0.5};
  const double brush_size{1.75};

  const auto cell{shared::cursor_to_cell(cursor_position,
                                         static_cast<float>(cursor_offset))};
  const double scaled_x{cell.x()};
  const double scaled_y{cell.y()};

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

  propagate();
}

void main_presenter::draw_image() {
  drawing_view_->draw_grid(drawn_image_);
}

void main_presenter::propagate() {
  Eigen::Map<Eigen::VectorXf> input{drawn_image_.data(),
                                    shared::GRID_SIZE * shared::GRID_SIZE};
  model_->predict(input);
}

} // namespace presenters
