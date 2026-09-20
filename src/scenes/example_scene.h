#pragma once

#include "engine/graphics/renderer.h"
#include "engine/input/event_bus.h"
#include "engine/scene.h"
#include "models/example.h"
#include "presenters/example_presenter.h"
#include "views/example_view_gl.h"
#include <memory>

namespace scenes {

class example_scene : public engine::scene {
public:
  example_scene(engine::graphics::renderer &renderer,
                engine::input::event_bus &bus)
      : view_(renderer),
        presenter_(std::make_unique<models::example>(model_),
                   std::make_unique<views::example_view_gl>(view_), bus) {
  }

  void on_render() override {
    presenter_.load_and_render();
  }

  void on_update() override {
  }

private:
  models::example model_;
  views::example_view_gl view_;
  presenters::example_presenter presenter_;
};

} // namespace scenes
