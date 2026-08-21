#include "layer.h"

#include "shared/math.h"

namespace mlp {

layer::layer(int n, int prev_n)
    : activations(n), z_values(n), biases(n), weights(n, prev_n) {
}

void layer::update(layer *previous_layer) {
  z_values = weights * previous_layer->activations + biases;
  if (is_output) {
    // https://en.wikipedia.org/wiki/Softmax_function
    // https://stackoverflow.com/questions/42599498/numerically-stable-softmax
    const auto max_z{z_values.maxCoeff()};
    activations = (z_values.array() - max_z).exp();
    activations /= activations.sum();
  } else {
    activations = z_values.unaryExpr([](float z) { return shared::relu(z); });
  }
}

int layer::n() const {
  return static_cast<int>(weights.rows());
}

} // namespace mlp
