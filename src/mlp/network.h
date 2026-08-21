#pragma once

#include <filesystem>
#include <vector>

#include <Eigen/Core>

#include "mlp/data_point.h"
#include "mlp/layer.h"

namespace mlp {

class network {
public:
  void to_file(const std::filesystem::path &file_path);

protected:
  explicit network(const std::vector<int> &layer_sizes);

  explicit network(const std::filesystem::path &file_path);

  /// Randomly initializes weights between each neuron using He initialization.
  ///
  /// @see https://en.wikipedia.org/wiki/Weight_initialization#He_initialization
  void initialize_weights();

  /// Performs a single backprogatation step with a list of training data.
  ///
  /// @see https://en.wikipedia.org/wiki/Backpropagation
  /// @see https://youtu.be/Ilg3gGewQ5U
  /// @see https://youtu.be/tIeHLnjs5U8
  float backpropagate(const std::vector<mlp::data_point> &training_batch);

  void set_input(const Eigen::VectorXf &input);
  void update();

  [[nodiscard]] const Eigen::VectorXf &output() const;

private:
  float backpropagate_once(const mlp::data_point &data_point);

  void initialize_layers(const std::vector<int> &layer_sizes);

  std::vector<layer> layers_;
  std::vector<layer> gradient_layers_;
  std::vector<layer> gradient_sum_layers_;
};

} // namespace mlp
