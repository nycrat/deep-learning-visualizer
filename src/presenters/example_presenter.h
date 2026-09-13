#pragma once

#include <memory>

#include "models/example.h"
#include "views/example_view.h"

namespace presenters {

class example_presenter {
public:
  example_presenter(std::unique_ptr<models::example> model,
                    std::unique_ptr<views::example_view> view)
      : view_(std::move(view)), model_(std::move(model)) {
  }

  void load_and_render();

private:
  std::unique_ptr<views::example_view> view_;
  std::unique_ptr<models::example> model_;
};

} // namespace presenters
