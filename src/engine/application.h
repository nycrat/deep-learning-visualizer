#pragma once

/// @file

#include <memory>
#include <utility>

#include "engine/graphics/renderer.h"
#include "engine/input/event_bus.h"
#include "engine/scene.h"
#include "engine/window.h"

namespace engine {

/**
 * Desktop GUI manager that handles the application lifecycle.
 *
 * This class owns the window manager, renderer, and input manager.
 */
class application {
public:
  /**
   * Runs the update and render loop with the current scene.
   *
   * This method is blocking.
   */
  void run();

  /**
   * Sets the active scene.
   *
   * @param scene A unique pointer to the new scene.
   *
   * @note The scene's ownership is transferred to application.
   */
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
