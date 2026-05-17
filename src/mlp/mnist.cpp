#include <cstddef>
#include <print>
#include <ranges>

#include "mnist.h"
#include "shared/idx_matrix.h"

namespace mlp {

mnist::mnist() : network({28 * 28, 64, 64, 64, 10}) {
  initialize_weights();

  shared::idx_matrix training_labels{"data/mnist/train-labels.idx"};
  shared::idx_matrix training_images{"data/mnist/train-images.idx"};

  std::size_t batch_size{1}; // TODO fix this
  std::size_t rows{training_labels.rows()};

  auto raw_inputs = training_images.data() | std::views::transform([](auto x) {
                      return static_cast<float>(x) / 255.0f;
                    }) |
                    std::ranges::to<std::vector<float>>();

  auto raw_outputs = training_labels.data() | std::views::transform([](auto x) {
                       std::array<float, 10> v{};
                       v[x] = 1.0f;
                       return v;
                     }) |
                     std::views::join | std::ranges::to<std::vector<float>>();

  for (std::size_t it{0}; it < 10; it++) {
    for (std::size_t i{0}; i < rows / batch_size; i++) {
      Eigen::Map<Eigen::MatrixXf> inputs(
          raw_inputs.data() + (i * batch_size * 28 * 28), batch_size, 28 * 28);
      Eigen::Map<Eigen::MatrixXf> outputs(
          raw_outputs.data() + (i * batch_size * 10), batch_size, 10);

      backpropagate(inputs, outputs);
    }
    std::println("Training {}/{} iterations complete", it + 1, 10);
  }

  shared::idx_matrix testing_labels{"data/mnist/test-labels.idx"};
  shared::idx_matrix testing_images{"data/mnist/test-images.idx"};

  auto raw_test_inputs = testing_images.data() |
                         std::views::transform([](auto x) {
                           return static_cast<float>(x) / 255.0f;
                         }) |
                         std::ranges::to<std::vector<float>>();

  int correct{0};
  for (int i = 0; i < testing_labels.rows(); i++) {
    Eigen::Map<Eigen::VectorXf> test_input(raw_test_inputs.data() + 28 * 28 * i,
                                           28 * 28);
    set_input(test_input);
    update();

    std::uint8_t prediction = 0;
    float largest = output()(0);
    for (std::uint8_t digit{0}; digit < 10; digit++) {
      if (output()(digit) > largest) {
        prediction = digit;
        largest = output()(digit);
      }
    }
    if (prediction == testing_labels.data()[i]) {
      correct++;
    }
  }

  std::println("{}% accuracy",
               static_cast<float>(correct) / testing_labels.rows() * 100.0f);
}

int mnist::predict(const Eigen::VectorXf &input) {
  set_input(input);
  update();

  auto output_vec = output();

  int predicted_digit{0};
  float max_activation{output_vec(0)};

  for (int digit{1}; digit < 10; digit++) {
    if (output_vec(digit) > max_activation) {
      predicted_digit = digit;
      max_activation = output_vec(digit);
    }
  }

  return predicted_digit;
}

} // namespace mlp
