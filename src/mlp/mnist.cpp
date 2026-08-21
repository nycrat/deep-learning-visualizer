#include <algorithm>
#include <print>
#include <random>
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
    : network({shared::TOTAL_PIXELS, 128, 128, shared::TOTAL_DIGITS}) {
  initialize_weights();
}

mnist::mnist(const std::filesystem::path &file_path) : network(file_path) {
}

void mnist::train() {
  const int epochs{10};
  const std::int64_t batch_size{64};
  const std::int64_t rows{training_labels_.rows()};

  const auto raw_inputs = to_float_vector(training_images_.data());
  const auto raw_outputs =
      training_labels_.data() | std::views::transform([](auto digit) {
        std::array<float, shared::TOTAL_DIGITS> v{};
        v.at(digit) = 1.0f;
        return v;
      }) |
      std::views::join | std::ranges::to<std::vector<float>>();

  static std::random_device rd{};
  static std::mt19937 gen{rd()};

  auto indices{std::views::iota(0, rows) | std::ranges::to<std::vector<int>>()};

  for (int epoch{}; epoch < epochs; epoch++) {
    std::shuffle(indices.begin(), indices.end(), gen);
    float total_cost{};
    for (std::int64_t i{}; i < rows; i += batch_size) {
      const auto safe_batch_size{std::min(batch_size, rows - i)};
      const auto batch_input_span =
          raw_inputs | std::views::drop(i * shared::TOTAL_PIXELS);
      const auto batch_output_span =
          raw_outputs | std::views::drop(i * shared::TOTAL_DIGITS);

      Eigen::Map<const Eigen::MatrixXf> inputs{
          batch_input_span.data(), safe_batch_size, shared::TOTAL_PIXELS};
      Eigen::Map<const Eigen::MatrixXf> outputs{
          batch_output_span.data(), safe_batch_size, shared::TOTAL_DIGITS};

      backpropagate(inputs, outputs);
    }
    std::println("Training {}/{} epochs complete; Current cost: ", epoch + 1,
                 epochs, total_cost);
  }
}

void mnist::test() {
  const auto raw_test_inputs = to_float_vector(testing_images_.data());

  int correct{};
  for (int i{}; i < testing_labels_.rows(); i++) {
    const auto test_input_span =
        raw_test_inputs | std::views::drop(shared::TOTAL_PIXELS * i);
    Eigen::Map<const Eigen::VectorXf> test_input{test_input_span.data(),
                                                 shared::TOTAL_PIXELS};
    set_input(test_input);
    update();

    int predicted_digit{};
    float largest{output()(0)};
    for (int digit{}; digit < shared::TOTAL_DIGITS; digit++) {
      if (output()(digit) > largest) {
        predicted_digit = digit;
        largest = output()(digit);
      }
    }
    if (predicted_digit ==
        static_cast<std::int8_t>(testing_labels_.data().at(i))) {
      correct++;
    }
  }

  std::println("{}% accuracy", 100.0f * static_cast<float>(correct) /
                                   static_cast<float>(testing_labels_.rows()));
}

int mnist::predict(const Eigen::VectorXf &input) {
  set_input(input);
  update();

  auto output_vec = output();

  std::println("Output: {}", output_vec);

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
