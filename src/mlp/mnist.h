#pragma once

#include "mlp/network.h"

namespace mlp {

class mnist : private network {
public:
  mnist();

  int predict(const Eigen::VectorXf &input);
};

} // namespace mlp
