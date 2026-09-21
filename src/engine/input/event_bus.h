#pragma once

#include "engine/input/constants.h"
#include <Eigen/Core>
#include <functional>
#include <vector>

namespace engine::input {

class event_bus {
public:
  void emit(key key, action action) {
    for (auto &func : key_callbacks_) {
      func(key, action);
    }
  }

  void emit(const Eigen::Vector2f &cursor_pos) {
    for (auto &func : cursor_callbacks_) {
      func(cursor_pos);
    }
  }

  void subscribe(std::function<void(key, action)> func) {
    key_callbacks_.push_back(func);
  }

  void subscribe(std::function<void(Eigen::Vector2f)> func) {
    cursor_callbacks_.push_back(func);
  }

private:
  std::vector<std::function<void(key, action)>> key_callbacks_;
  std::vector<std::function<void(Eigen::Vector2f)>> cursor_callbacks_;
};

} // namespace engine::input
