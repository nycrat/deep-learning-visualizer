#pragma once

#include <cstddef>
#include <span>
#include <vector>

#include <Eigen/Core>

#include "layer.h"

namespace mlp {

class network {
public:
  network(std::span<const std::size_t> layer_sizes);

  /// Randomly initializes weights between each neuron using Uniform Xavier Initialization.
  ///
  /// @see https://en.wikipedia.org/wiki/Weight_initialization#Glorot_initialization
  void initialize_weights();

  /// Performs a single backprogatation step with a list of training data.
  /// TODO: refactor to pass in inputs and outputs as one argument with a list of structs
  ///
  /// @see https://en.wikipedia.org/wiki/Backpropagation
  /// @see https://youtu.be/Ilg3gGewQ5U
  /// @see https://youtu.be/tIeHLnjs5U8
  void backpropagate(std::span<const Eigen::VectorXf> inputs,
                     std::span<const Eigen::VectorXf> outputs);

  void update();

  const Eigen::VectorXf &output() const;

private:
  void backpropagate_once(const Eigen::VectorXf &input,
                          const Eigen::VectorXf &output);

  std::vector<layer> layers_{};
  std::vector<layer> gradient_layers_{};
};

} // namespace mlp
