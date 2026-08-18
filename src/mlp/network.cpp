#include "network.h"

#include <cmath>
#include <cstdint>
#include <format>
#include <fstream>
#include <random>
#include <ranges>
#include <stdexcept>
#include <vector>

#include "shared/math.h"

namespace {

float get_normal(float mean, float standard_deviation) {
  static std::random_device rd{};
  static std::mt19937 gen{rd()};
  std::normal_distribution distr(mean, standard_deviation);
  return distr(gen);
}

const float learning_rate = 0.05f;

} // namespace

namespace mlp {

network::network(const std::vector<int> &layer_sizes) {
  initialize_layers(layer_sizes);
}

network::network(const std::filesystem::path &file_path) {
  std::ifstream file{file_path};

  if (!file) {
    throw std::runtime_error(
        std::format("Failed to open {}", file_path.string()));
  }

  int num_layers{};
  file >> num_layers;

  std::vector<int> layer_sizes(num_layers);

  for (auto &layer_size : layer_sizes) {
    file >> layer_size;
  }

  initialize_layers(layer_sizes);

  for (auto &layer : layers_ | std::views::drop(1)) {
    for (auto &bias : layer.biases) {
      file >> bias;
    }
  }

  for (auto &layer : layers_ | std::views::drop(1)) {
    for (std::int64_t i{0}; i < layer.weights.rows(); i++) {
      for (auto &weight : layer.weights.row(i)) {
        file >> weight;
      }
    }
  }
}

void network::to_file(const std::filesystem::path &file_path) {
  std::ofstream file{file_path};

  file << layers_.size() << "\n";

  for (const auto &layer : layers_) {
    file << layer.n() << " ";
  }

  file << "\n\n";

  for (const auto &layer : layers_ | std::views::drop(1)) {
    for (auto bias : layer.biases) {
      file << bias << " ";
    }
    file << "\n";
  }

  file << "\n";

  for (const auto &layer : layers_ | std::views::drop(1)) {
    for (const auto &row : layer.weights.rowwise()) {
      for (auto weight : row) {
        file << weight << " ";
      }
      file << "\n";
    }
  }
}

void network::initialize_weights() {
  for (auto &&[prev, cur] : layers_ | std::views::adjacent<2>) {
    const auto inputs{prev.n()};

    const auto sd{std::sqrtf(2.0f / static_cast<float>(inputs))};

    cur.weights = Eigen::MatrixXf::NullaryExpr(
        cur.n(), prev.n(), [sd]() { return get_normal(0, sd); });
  }
}

void network::backpropagate(const Eigen::MatrixXf &inputs,
                            const Eigen::MatrixXf &outputs) {
  for (auto &gradient_layer : gradient_layers_) {
    gradient_layer.biases.setZero();
    gradient_layer.weights.setZero();
  }

  for (std::int64_t i{0}; i < inputs.rows(); i++) {
    backpropagate_once(inputs.row(i), outputs.row(i));
  }

  for (auto &&[layer, gradient] : std::views::zip(layers_, gradient_layers_)) {
    layer.biases -= learning_rate * gradient.biases / inputs.rows();
    layer.weights -= learning_rate * gradient.weights / inputs.rows();
  }
}

void network::set_input(const Eigen::VectorXf &input) {
  layers_.front().activations = input;
}

void network::update() {
  for (auto &&[prev, layer] : layers_ | std::views::adjacent<2>) {
    layer.update(&prev);
  }
}

const Eigen::VectorXf &network::output() const {
  return layers_.back().activations;
}

void network::backpropagate_once(const Eigen::VectorXf &input,
                                 const Eigen::VectorXf &output) {
  layers_.front().activations = input;
  update();

  // for first layer, dc_da = 2 * (a - y)
  gradient_layers_.back().activations =
      2 * (layers_.back().activations - output);

  for (std::int64_t i{std::ssize(layers_) - 1}; i >= 1; i--) {
    auto &layer = layers_.at(i);
    auto &layer_prev = layers_.at(i - 1);
    auto &grad = gradient_layers_.at(i);
    auto &grad_prev = gradient_layers_.at(i - 1);

    auto dc_da = grad.activations;
    auto da_dz =
        layer.z_values.unaryExpr([](float z) { return shared::d_sigmoid(z); });
    Eigen::VectorXf delta = dc_da.array() * da_dz.array();

    // calcuate gradient for weight and bias for current layer
    grad.weights.noalias() += delta * layer_prev.activations.transpose();
    grad.biases += delta;

    // calculate gradient for neuron activation of previous layer
    grad_prev.activations.noalias() = layer.weights.transpose() * delta;
  }
}

void network::initialize_layers(const std::vector<int> &layer_sizes) {
  if (layer_sizes.size() < 2) {
    throw std::runtime_error("Neural network should have at least two layers");
  }

  int prev{0};
  for (auto n : layer_sizes) {
    layers_.emplace_back(n, prev);
    gradient_layers_.emplace_back(n, prev);
    prev = n;
  }

  layers_.back().is_output = true;
  gradient_layers_.back().is_output = true;
}

} // namespace mlp
