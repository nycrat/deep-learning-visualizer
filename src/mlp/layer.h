#pragma once

#include <Eigen/Core>

namespace mlp {

struct layer {
  layer(int n, int prev_n);

  void update(layer *previous_layer);

  [[nodiscard]] int n() const;

  Eigen::VectorXf activations;
  Eigen::VectorXf z_values;
  Eigen::VectorXf biases;
  Eigen::MatrixXf weights;
  bool is_output{false};
};

} // namespace mlp
