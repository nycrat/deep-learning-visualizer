#include "presenters/example_presenter.h"

namespace presenters {

void example_presenter::load_and_render() {
  view_->display_square(model_->square_position);
  view_->display_triangle(model_->triangle_position);
}

} // namespace presenters
