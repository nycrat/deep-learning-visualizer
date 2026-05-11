#include "network.h"

#include <algorithm>
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

const float learning_rate = 0.05f;

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

void network::backpropagate(std::span<const Eigen::VectorXf> inputs,
                            std::span<const Eigen::VectorXf> outputs) {
  for (auto &gradient_layer : gradient_layers_) {
    gradient_layer.activations.setZero();
    gradient_layer.biases.setZero();
    gradient_layer.weights.setZero();
  }

  for (auto &&[input, output] : std::views::zip(inputs, outputs)) {
    backpropagate_once(input, output);
  }

  for (auto &&[layer, gradient] : std::views::zip(layers_, gradient_layers_)) {
    layer.biases -= learning_rate * gradient.biases / inputs.size();
    layer.weights -= learning_rate * gradient.weights / inputs.size();
  }
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
  gradient_layers_.back().activations = output;
  layers_.front().activations = input;
  update();

  for (std::size_t i{layers_.size() - 1}; i >= 1; i--) {
    auto &layer = layers_[i];
    auto &layer_prev = layers_[i - 1];
    auto &gradient_layer = gradient_layers_[i];
    auto &gradient_layer_prev = gradient_layers_[i - 1];

    // calcuate gradient for weight and bias for current layer
    for (std::size_t j{0}; j < layer.n(); j++) {
      float a = layer.activations(j);
      float y = gradient_layer.activations(j);
      float z = shared::inv_sigmoid(a);

      float dc_da = 2 * (a - y);
      float da_dz = shared::d_sigmoid(z);

      for (std::size_t k{0}; k < layer_prev.n(); k++) {
        float dz_dw = layer_prev.activations(k);
        float dc_dw = dc_da * da_dz * dz_dw;

        gradient_layer.weights(j, k) += dc_dw;
      }

      float dz_db = 1.0f;
      float dc_db = dc_da * da_dz * dz_db;
      gradient_layer.biases(j) += dc_db;
    }

    // calculate gradient for neuron activation of previous layer
    for (std::size_t k{0}; k < layer_prev.n(); k++) {
      float dc_da_minus = 0.0f;
      for (std::size_t j{0}; j < layer.n(); j++) {
        float a = layer.activations(j);
        float y = gradient_layer.activations(j);
        float z = shared::inv_sigmoid(a);

        float dc_da = 2 * (a - y);
        float da_dz = shared::d_sigmoid(z);
        float dz_da_minus = layer.weights(j, k);
        dc_da_minus += dc_da * da_dz * dz_da_minus;
      }
      gradient_layer_prev.activations(k) =
          layer_prev.activations(k) - learning_rate * dc_da_minus;
    }
  }
}

} // namespace mlp
