#pragma once

/** @file */

#include "engine/graphics/renderer.h"
#include "engine/input/event_bus.h"
#include "engine/scene.h"
#include "models/mnist.h"
#include "presenters/pixel_grid_presenter.h"
#include "views/pixel_grid_view.h"
#include <memory>

namespace scenes {

class pixel_grid_scene : public engine::scene {
public:
  pixel_grid_scene(engine::graphics::renderer &renderer,
                   engine::input::event_bus &bus)
      : presenter_(std::make_unique<models::mnist>("data/mnist/trained.mlp"),
                   std::make_unique<views::pixel_grid_view>(renderer), bus) {
  }

  void render() override {
    presenter_.render();
  }

  void update() override {
  }

private:
  presenters::pixel_grid_presenter presenter_;
};

} // namespace scenes
