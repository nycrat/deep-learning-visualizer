#include "catch_amalgamated.h"
#include "mlp/layer.h"

TEST_CASE("Test layer constructor") {
  mlp::layer l{7, 5};

  CHECK(l.n() == 7);
  CHECK(l.activations.rows() == 7);
  CHECK(l.activations.cols() == 1);
  CHECK(l.z_values.rows() == 7);
  CHECK(l.z_values.cols() == 1);
  CHECK(l.biases.rows() == 7);
  CHECK(l.biases.cols() == 1);
  CHECK(l.weights.rows() == 7);
  CHECK(l.weights.cols() == 5);
}
