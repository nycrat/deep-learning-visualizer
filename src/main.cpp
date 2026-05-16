#include <array>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <print>
#include <ranges>
#include <vector>

#include <Eigen/Core>

#include "mlp/network.h"
#include "shared/idx_matrix.h"
#include "ui/application.h"

namespace {

// TODO remove later
void test_mlp() {
  std::vector<std::size_t> config{28 * 28, 64, 64, 64, 10};
  // std::vector<std::size_t> config{28 * 28, 28 * 14, 28 * 7, 14 * 7, 7 * 7, 20, 10};
  mlp::network network{config};
  network.initialize_weights();

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

      network.backpropagate(inputs, outputs);
      std::println("{}: {}", it * rows / batch_size + i, network.output());
    }
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
    network.set_input(test_input);
    network.update();

    std::uint8_t prediction = 0;
    float largest = network.output()(0);
    for (std::uint8_t digit{0}; digit < 10; digit++) {
      if (network.output()(digit) > largest) {
        prediction = digit;
        largest = network.output()(digit);
      }
    }
    if (prediction == testing_labels.data()[i]) {
      correct++;
    } else {
      std::println("Wrong index {}: {} {}", i, prediction,
                   testing_labels.data()[i]);
    }
  }

  std::println("{}% accuracy",
               static_cast<float>(correct) / testing_labels.rows() * 100.0f);
}

} // namespace

int main() {
  test_mlp();
  try {
    ui::application app{};
    app.run();
  } catch (const std::exception &e) {
    std::println(stderr, "Runtime error: {}", e.what());
    return 1;
  }
}
