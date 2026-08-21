#pragma once

#include "mlp/network.h"
#include "shared/idx_matrix.h"

#include <filesystem>

namespace mlp {

class mnist : public network {
public:
  mnist();
  explicit mnist(const std::filesystem::path &file_path);

  void train();
  void test();
  int predict(const Eigen::VectorXf &input);

private:
  inline static const shared::idx_matrix training_labels_{
      "data/mnist/train-labels.idx"};
  inline static const shared::idx_matrix training_images_{
      "data/mnist/train-images.idx"};

  inline static const shared::idx_matrix testing_labels_{
      "data/mnist/test-labels.idx"};
  inline static const shared::idx_matrix testing_images_{
      "data/mnist/test-images.idx"};
};

} // namespace mlp
