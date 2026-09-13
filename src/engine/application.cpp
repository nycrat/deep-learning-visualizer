#include "engine/application.h"

namespace engine {

void application::run() {
  while (!window_.should_close()) {
    window_.wait_events();
    if (current_scene_) {
      current_scene_->on_update();
      renderer_.begin_frame();
      current_scene_->on_render();
      renderer_.end_frame();
    }
    window_.swap_buffers();
  }
}

} // namespace engine
