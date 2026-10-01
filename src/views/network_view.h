#pragma once

/** @file */

#include "models/layer.h"

#include <vector>

namespace views {

/**
 * An abstract interface for drawing the internals of a neural network.
 */
class network_view {
public:
  virtual ~network_view() = default;

  /**
   * Draws every layer as a column of neurons, with a connection between each
   * pair of adjacent layers.
   *
   * @param layers The layers of the network, ordered from input to output.
   */
  virtual void draw_network(const std::vector<models::layer> &layers) = 0;
};

} // namespace views
