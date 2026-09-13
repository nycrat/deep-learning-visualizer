#pragma once

#include <memory>
#include <utility>

#include "engine/graphics/renderer.h"
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

private:
  window window_{};
  graphics::renderer renderer_{};
  std::unique_ptr<scene> current_scene_{};
};

} // namespace engine
