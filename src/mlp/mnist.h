#pragma once

#include "mlp/network.h"

#include <filesystem>

namespace mlp {

class mnist : public network {
public:
  mnist();
  explicit mnist(const std::filesystem::path &file_path);

  void train();
  int predict(const Eigen::VectorXf &input);
};

} // namespace mlp
