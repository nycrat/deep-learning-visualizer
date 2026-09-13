#pragma once

namespace engine {

class scene {
public:
  virtual ~scene() = default;

  virtual void on_update() = 0;
  virtual void on_render() = 0;
};

} // namespace engine
