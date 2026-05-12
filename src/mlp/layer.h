#pragma once

#include <cstddef>

#include <Eigen/Core>

namespace mlp {

struct layer {
  layer(std::size_t n, std::size_t prev_n);

  void update(layer *previous_layer);

  std::size_t n() const;

  Eigen::VectorXf activations{};
  Eigen::VectorXf z_values{};
  Eigen::VectorXf biases{};
  Eigen::MatrixXf weights{};
};

} // namespace mlp
