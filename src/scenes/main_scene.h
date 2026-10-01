#pragma once

/** @file */

#include "engine/graphics/renderer.h"
#include "engine/input/event_bus.h"
#include "engine/scene.h"
#include "models/mnist.h"
#include "presenters/main_presenter.h"
#include "views/network_view_gl.h"
#include "views/pixel_grid_view_gl.h"
#include <memory>

namespace scenes {

class main_scene : public engine::scene {
public:
  main_scene(engine::graphics::renderer &renderer,
             engine::input::event_bus &bus)
      : presenter_(std::make_unique<models::mnist>("data/mnist/trained.mlp"),
                   std::make_unique<views::pixel_grid_view_gl>(renderer),
                   std::make_unique<views::network_view_gl>(renderer), bus) {
  }

  void render() override {
    presenter_.render();
  }

  void update() override {
  }

private:
  presenters::main_presenter presenter_;
};

} // namespace scenes
