#pragma once

#include "engine/input/constants.h"
#include <Eigen/Core>
#include <functional>
#include <vector>

namespace engine::input {

/**
 * An event dispatcher for input events.
 */
class event_bus {
public:
  /**
   * Informs all subscribers of a new keyboard event.
   *
   * @param key The key whose status is changed.
   * @param action The specific action that is performed.
   */
  void emit(key key, action action) {
    for (auto &func : key_callbacks_) {
      func(key, action);
    }
  }

  /**
   * Informs all subscribers of a new cursor movement event.
   *
   * @param cursor_pos The new cursor position.
   */
  void emit(const Eigen::Vector2f &cursor_pos) {
    for (auto &func : cursor_callbacks_) {
      func(cursor_pos);
    }
  }

  /**
   * Attaches a subscriber to new keyboard events.
   *
   * @param func A function to be called when an event is emitted.
   */
  void subscribe(std::function<void(key, action)> func) {
    key_callbacks_.push_back(func);
  }

  /**
   * Attaches a subscriber to new cursor movement events.
   *
   * @param func A function to be called when an event is emitted.
   */
  void subscribe(std::function<void(Eigen::Vector2f)> func) {
    cursor_callbacks_.push_back(func);
  }

private:
  std::vector<std::function<void(key, action)>> key_callbacks_;
  std::vector<std::function<void(Eigen::Vector2f)>> cursor_callbacks_;
};

} // namespace engine::input
