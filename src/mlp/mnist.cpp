#include <print>
#include <ranges>

#include "mnist.h"
#include "shared/idx_matrix.h"
#include "ui/application.h"

namespace {

std::vector<float> to_float_vector(std::span<const uint8_t> data) {
  const float max_value{255.0f};
  return data | std::views::transform([&](auto x) {
           return static_cast<float>(x) / max_value;
         }) |
         std::ranges::to<std::vector<float>>();
}

} // namespace

namespace mlp {

mnist::mnist()
    : network({shared::TOTAL_PIXELS, 64, 64, 64, shared::TOTAL_DIGITS}) {
  initialize_weights();

  shared::idx_matrix training_labels{"data/mnist/train-labels.idx"};
  shared::idx_matrix training_images{"data/mnist/train-images.idx"};

  std::int64_t batch_size{1}; // TODO fix this
  std::int64_t rows{training_labels.rows()};

  auto raw_inputs = to_float_vector(training_images.data());

  auto raw_outputs = training_labels.data() |
                     std::views::transform([](auto digit) {
                       std::array<float, shared::TOTAL_DIGITS> v{};
                       v.at(digit) = 1.0f;
                       return v;
                     }) |
                     std::views::join | std::ranges::to<std::vector<float>>();

  const int iterations = 10;

  for (int it{0}; it < iterations; it++) {
    for (std::int64_t i{0}; i < rows / batch_size; i++) {
      auto batch_inputs =
          raw_inputs | std::views::drop(i * batch_size * shared::TOTAL_PIXELS);
      Eigen::Map<Eigen::MatrixXf> inputs(batch_inputs.data(), batch_size,
                                         shared::TOTAL_PIXELS);

      auto batch_outputs =
          raw_outputs | std::views::drop(i * batch_size * shared::TOTAL_DIGITS);
      Eigen::Map<Eigen::MatrixXf> outputs(batch_outputs.data(), batch_size,
                                          shared::TOTAL_DIGITS);

      backpropagate(inputs, outputs);
    }
    std::println("Training {}/{} iterations complete", it + 1, iterations);
  }

  shared::idx_matrix testing_labels{"data/mnist/test-labels.idx"};
  shared::idx_matrix testing_images{"data/mnist/test-images.idx"};

  auto raw_test_inputs = to_float_vector(testing_images.data());

  int correct{0};
  for (int i = 0; i < testing_labels.rows(); i++) {
    auto test_input_span =
        raw_test_inputs | std::views::drop(shared::TOTAL_PIXELS * i);
    Eigen::Map<Eigen::VectorXf> test_input(test_input_span.data(),
                                           shared::TOTAL_PIXELS);
    set_input(test_input);
    update();

    int prediction = 0;
    float largest = output()(0);
    for (int digit{0}; digit < shared::TOTAL_DIGITS; digit++) {
      if (output()(digit) > largest) {
        prediction = digit;
        largest = output()(digit);
      }
    }
    if (prediction == testing_labels.data().at(i)) {
      correct++;
    }
  }

  std::println("{}% accuracy", 100.0f * static_cast<float>(correct) /
                                   static_cast<float>(testing_labels.rows()));
}

int mnist::predict(const Eigen::VectorXf &input) {
  set_input(input);
  update();

  auto output_vec = output();

  int predicted_digit{0};
  float max_activation{output_vec(0)};

  for (int digit{1}; digit < shared::TOTAL_DIGITS; digit++) {
    if (output_vec(digit) > max_activation) {
      predicted_digit = digit;
      max_activation = output_vec(digit);
    }
  }

  return predicted_digit;
}

} // namespace mlp
