#include "network.h"

#include <cmath>
#include <random>
#include <ranges>
#include <span>
#include <stdexcept>

#include "shared/math.h"

namespace {

float get_random(float min = 0.0f, float max = 1.0f) {
  static std::random_device rd{};
  static std::mt19937 gen{rd()};
  std::uniform_real_distribution<> distr{min, max};
  return distr(gen);
}

const float learning_rate = 0.002f;

} // namespace

namespace mlp {

network::network(std::span<const std::size_t> layer_sizes) {
  if (layer_sizes.size() < 2) {
    throw std::runtime_error("Neural network should have at least two layers");
  }

  std::size_t prev{0};
  for (auto n : layer_sizes) {
    layers_.emplace_back(n, prev);
    gradient_layers_.emplace_back(n, prev);
    prev = n;
  }
}

void network::initialize_weights() {
  for (auto &&[prev, cur] : layers_ | std::views::adjacent<2>) {
    auto inputs = prev.n();
    auto outputs = cur.n();
    auto x = std::sqrt(6.0f / static_cast<float>(inputs + outputs));

    cur.weights = Eigen::MatrixXf::NullaryExpr(
        cur.n(), prev.n(), [x]() { return get_random(-x, x); });
  }
}

void network::backpropagate(const Eigen::MatrixXf &inputs,
                            const Eigen::MatrixXf &outputs) {
  for (auto &gradient_layer : gradient_layers_) {
    gradient_layer.biases.setZero();
    gradient_layer.weights.setZero();
  }

  for (std::size_t i{0}; i < inputs.rows(); i++) {
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

  for (std::size_t i{layers_.size() - 1}; i >= 1; i--) {
    auto &layer = layers_[i];
    auto &layer_prev = layers_[i - 1];
    auto &grad = gradient_layers_[i];
    auto &grad_prev = gradient_layers_[i - 1];

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

} // namespace mlp
