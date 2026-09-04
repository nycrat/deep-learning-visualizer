#pragma once

#include <Eigen/Core>

namespace models {

/**
  * A lightweight wrapper struct with a reference to an input and output row.
  */
struct data_point {
  Eigen::Map<const Eigen::VectorXf> input;
  Eigen::Map<const Eigen::VectorXf> output;
};

} // namespace models
