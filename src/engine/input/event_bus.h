#pragma once

#include "engine/input/constants.h"
#include <functional>
#include <vector>

namespace engine::input {

class event_bus {
public:
  void emit(int key, int action) {
    for (auto &func : callbacks) {
      func(static_cast<enum key>(key), static_cast<enum action>(action));
    }
  }

  void subscribe(std::function<void(key, action)> func) {
    callbacks.push_back(func);
  }

private:
  std::vector<std::function<void(key, action)>> callbacks;
};

} // namespace engine::input
