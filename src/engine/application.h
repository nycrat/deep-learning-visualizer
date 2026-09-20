#pragma once

#include <memory>
#include <utility>

#include "engine/graphics/renderer.h"
#include "engine/input/event_bus.h"
#include "engine/scene.h"
#include "engine/window.h"

namespace engine {

class application {
public:
  void run();
  void set_scene(std::unique_ptr<scene> scene) {
    current_scene_ = std::move(scene);
  }

  graphics::renderer &renderer() {
    return renderer_;
  };

  input::event_bus &event_bus() {
    return bus_;
  };

private:
  input::event_bus bus_{};
  window window_{bus_};
  graphics::renderer renderer_{};
  std::unique_ptr<scene> current_scene_{};
};

} // namespace engine
