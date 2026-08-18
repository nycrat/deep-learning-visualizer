#include "layer.h"

#include "shared/math.h"

namespace mlp {

layer::layer(int n, int prev_n)
    : activations(n), z_values(n), biases(n), weights(n, prev_n) {
}

void layer::update(layer *previous_layer) {
  z_values = weights * previous_layer->activations + biases;
  activations = z_values.unaryExpr([](float z) { return shared::relu(z); });
}

int layer::n() const {
  return static_cast<int>(weights.rows());
}

} // namespace mlp
