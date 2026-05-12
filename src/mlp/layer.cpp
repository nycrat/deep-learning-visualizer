#include "layer.h"

#include "shared/math.h"

namespace mlp {

layer::layer(std::size_t n, std::size_t prev_n)
    : activations(n), z_values(n), biases(n), weights(n, prev_n) {
}

void layer::update(layer *previous_layer) {
  z_values = weights * previous_layer->activations + biases;
  activations = z_values.unaryExpr([](float z) { return shared::sigmoid(z); });
}

std::size_t layer::n() const {
  return weights.rows();
}

} // namespace mlp
