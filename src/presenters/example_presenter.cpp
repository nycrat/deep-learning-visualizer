#include "presenters/example_presenter.h"

namespace presenters {

void example_presenter::load_and_render() {
  view_->display_square(model_->square_position);
  view_->display_line(model_->triangle_position, model_->square_position);
  view_->display_triangle(model_->triangle_position);
}

void example_presenter::handle_key_event(engine::input::key key,
                                         engine::input::action action) {
  if (action == engine::input::action::down) {
    switch (key) {
    case engine::input::key::h:
      model_->square_position.x() -= SQUARE_SPEED;
      break;
    case engine::input::key::j:
      model_->square_position.y() -= SQUARE_SPEED;
      break;
    case engine::input::key::k:
      model_->square_position.y() += SQUARE_SPEED;
      break;
    case engine::input::key::l:
      model_->square_position.x() += SQUARE_SPEED;
      break;
    default:
    }
  }
}

void example_presenter::handle_cursor_event(
    const Eigen::Vector2f &cursor_position) {
  model_->triangle_position = cursor_position;
}

} // namespace presenters
