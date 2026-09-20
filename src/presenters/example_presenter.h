#pragma once

#include <functional>
#include <memory>
#include <print>

#include "engine/input/constants.h"
#include "engine/input/event_bus.h"
#include "models/example.h"
#include "views/example_view.h"

namespace presenters {

class example_presenter {
public:
  example_presenter(std::unique_ptr<models::example> model,
                    std::unique_ptr<views::example_view> view,
                    engine::input::event_bus &bus)
      : view_(std::move(view)), model_(std::move(model)), bus_(bus) {
    bus.subscribe([](engine::input::key key, engine::input::action action) {
      if (action == engine::input::action::down) {
        std::println("{}", static_cast<int>(key));
      }
    });
  }

  void load_and_render();

private:
  std::unique_ptr<views::example_view> view_;
  std::unique_ptr<models::example> model_;
  std::reference_wrapper<engine::input::event_bus> bus_;
};

} // namespace presenters
