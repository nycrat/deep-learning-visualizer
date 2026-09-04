#pragma once

#include "models/network.h"
#include "shared/idx_matrix.h"

#include <filesystem>

namespace models {

class mnist : public network {
public:
  mnist();
  explicit mnist(const std::filesystem::path &file_path);

  void train();
  void test();
  int predict(const Eigen::VectorXf &input);

private:
  static const shared::idx_matrix &training_labels() {
    static shared::idx_matrix training_labels{"data/mnist/train-labels.idx"};
    return training_labels;
  }
  static const shared::idx_matrix &training_images() {
    static shared::idx_matrix training_images{"data/mnist/train-images.idx"};
    return training_images;
  }
  static const shared::idx_matrix &testing_labels() {
    static shared::idx_matrix testing_labels{"data/mnist/test-labels.idx"};
    return testing_labels;
  }
  static const shared::idx_matrix &testing_images() {
    static shared::idx_matrix testing_images{"data/mnist/test-images.idx"};
    return testing_images;
  }
};

} // namespace models
