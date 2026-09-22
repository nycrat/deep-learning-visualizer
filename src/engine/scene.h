#pragma once

/// @file

namespace engine {

/**
 * An abstract interface representing a complete application scene.
 */
class scene {
public:
  virtual ~scene() = default;

  /**
   * Performs non-event related updates.
   */
  virtual void update() = 0;

  /**
   * Renders all the scene content.
   */
  virtual void render() = 0;
};

} // namespace engine
