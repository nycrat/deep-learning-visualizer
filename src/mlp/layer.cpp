#include "layer.h"

#include "shared/math.h"

#include <cmath>

namespace mlp {

layer::layer(int n, int prev_n)
    : activations(n), z_values(n), biases(n), weights(n, prev_n) {
}

void layer::update(layer *previous_layer) {
  z_values = weights * previous_layer->activations + biases;
  if (is_output) {
    // https://en.wikipedia.org/wiki/Softmax_function
    activations = z_values.unaryExpr([](float z) { return std::exp(z); });
    activations /= activations.sum();
  } else {
    activations = z_values.unaryExpr([](float z) { return shared::relu(z); });
  }
}

int layer::n() const {
  return static_cast<int>(weights.rows());
}

} // namespace mlp
