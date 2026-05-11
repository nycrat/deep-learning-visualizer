#include "layer.h"

#include "shared/math.h"

namespace mlp {

layer::layer(std::size_t n, std::size_t prev_n)
    : activations(n), biases(n), weights(n, prev_n) {
}

void layer::update(layer *previous_layer) {
  activations =
      (weights * previous_layer->activations + biases).unaryExpr([](float a) {
        return shared::sigmoid(a);
      });
}

std::size_t layer::n() const {
  return weights.rows();
}

} // namespace mlp
