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
      std::println("{}", propagate());
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

  const float cursor_offset{0.5f};
  const float brush_size{1.75f};

  const auto cell{shared::cursor_to_cell(cursor_position, cursor_offset)};

  for (int i{0}; i < shared::GRID_SIZE; i++) {
    for (int j{0}; j < shared::GRID_SIZE; j++) {
      const auto dist_x{static_cast<float>(i) - cell.x()};
      const auto dist_y{static_cast<float>(j) - cell.y()};
      const auto distance{sqrt(dist_x * dist_x + dist_y * dist_y)};
      const auto pixel_index = i + j * shared::GRID_SIZE;

      drawn_image_.at(pixel_index) =
          std::max(-std::powf(distance / brush_size, 3) + 1.0f,
                   drawn_image_.at(pixel_index));
    }
  }

  propagate();
}

void main_presenter::draw_image() {
  drawing_view_->draw_grid(drawn_image_);
}

int main_presenter::propagate() {
  Eigen::Map<Eigen::VectorXf> input{drawn_image_.data(),
                                    shared::GRID_SIZE * shared::GRID_SIZE};
  return model_->predict(input);
}

} // namespace presenters
