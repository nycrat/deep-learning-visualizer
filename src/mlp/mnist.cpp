#include <algorithm>
#include <print>
#include <random>
#include <ranges>

#include "mlp/data_point.h"
#include "mlp/mnist.h"
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
  const int epochs{5};
  const std::int64_t batch_size{64};
  const std::int64_t rows{training_labels().rows()};

  const auto raw_inputs = to_float_vector(training_images().data());
  const auto raw_outputs =
      training_labels().data() | std::views::transform([](auto digit) {
        std::array<float, shared::TOTAL_DIGITS> v{};
        v.at(digit) = 1.0f;
        return v;
      }) |
      std::views::join | std::ranges::to<std::vector<float>>();

  static std::random_device rd{};
  static std::mt19937 gen{rd()};

  auto indices{std::views::iota(0, rows) | std::ranges::to<std::vector<int>>()};

  std::vector<mlp::data_point> batch;
  batch.reserve(batch_size);

  for (int epoch{1}; epoch <= epochs; epoch++) {
    std::shuffle(indices.begin(), indices.end(), gen);
    float total_cost{};
    for (std::int64_t start{}; start < rows; start += batch_size) {
      const auto safe_batch_size{std::min(batch_size, rows - start)};

      for (std::int64_t k{}; k < safe_batch_size; k++) {
        const auto index{indices.at(start + k)};
        const auto batch_input{
            std::span{raw_inputs}.subspan(index * shared::TOTAL_PIXELS)};
        const auto batch_output{
            std::span{raw_outputs}.subspan(index * shared::TOTAL_DIGITS)};

        batch.emplace_back(
            Eigen::Map<const Eigen::VectorXf>{batch_input.data(),
                                              shared::TOTAL_PIXELS},
            Eigen::Map<const Eigen::VectorXf>{batch_output.data(),
                                              shared::TOTAL_DIGITS});
      }

      total_cost += backpropagate(batch);
      batch.clear();
    }
    std::println("Training {}/{} epochs complete; Current cost: {}", epoch,
                 epochs, total_cost);
  }
}

void mnist::test() {
  const auto raw_test_inputs = to_float_vector(testing_images().data());

  int correct{};
  for (int i{}; i < testing_labels().rows(); i++) {
    const auto test_input_span =
        std::span{raw_test_inputs}.subspan(shared::TOTAL_PIXELS * i);
    Eigen::Map<const Eigen::VectorXf> test_input{test_input_span.data(),
                                                 shared::TOTAL_PIXELS};
    if (predict(test_input) ==
        static_cast<std::int8_t>(testing_labels().data().at(i))) {
      correct++;
    }
  }

  std::println("{}% accuracy", 100.0f * static_cast<float>(correct) /
                                   static_cast<float>(testing_labels().rows()));
}

int mnist::predict(const Eigen::VectorXf &input) {
  set_input(input);
  update();

  const auto output_vec{output()};

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
