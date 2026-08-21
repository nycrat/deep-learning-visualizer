#pragma once

#include <Eigen/Core>

namespace mlp {

/**
  * A lightweight wrapper struct with a reference to a input and output row.
  */
struct data_point {
  Eigen::Map<const Eigen::VectorXf> input;
  Eigen::Map<const Eigen::VectorXf> output;
};

} // namespace mlp
