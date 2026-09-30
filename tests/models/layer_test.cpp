#include "catch_amalgamated.h"
#include "models/layer.h"
#include "shared/math.h"
#include "test_helpers.h"

#include <Eigen/Core>

#include <cmath>

namespace {

/**
 * Creates a layer that only serves as the source of activations for the layer
 * under test.
 */
models::layer make_input_layer(const Eigen::VectorXf &activations) {
  models::layer input{static_cast<int>(activations.size()), 0};
  input.activations = activations;
  return input;
}

Eigen::VectorXf vec(std::initializer_list<float> values) {
  return Eigen::VectorXf{values};
}

} // namespace

TEST_CASE("Test layer constructor") {
  models::layer l{7, 5};

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

TEST_CASE("Test layer constructor zero initializes its values") {
  models::layer l{3, 2};

  CHECK(l.activations.isZero(0.0f));
  CHECK(l.z_values.isZero(0.0f));
  CHECK(l.biases.isZero(0.0f));
  CHECK(l.weights.isZero(0.0f));
  CHECK(l.is_output == false);
}

TEST_CASE("Test layer update computes the weighted sum of its inputs") {
  auto input{make_input_layer(vec({1.0f, 2.0f}))};

  models::layer l{2, 2};
  l.weights << 1.0f, 0.0f, 0.0f, 1.0f;
  l.biases << 0.0f, -1.0f;

  l.update(&input);

  CHECK(l.z_values(0) == 1.0f);
  CHECK(l.z_values(1) == 1.0f);
}

TEST_CASE("Test layer update applies relu to a hidden layer") {
  auto input{make_input_layer(vec({1.0f, -2.0f, 3.0f}))};

  models::layer l{2, 3};
  l.weights << 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f;
  l.biases << 0.0f, 0.0f;

  l.update(&input);

  CHECK(l.activations(0) == 1.0f);
  CHECK(l.activations(1) == 0.0f);
}

TEST_CASE("Test layer update applies relu elementwise") {
  auto input{make_input_layer(vec({1.0f, -1.0f}))};

  models::layer l{2, 2};
  l.weights << 1.0f, 0.0f, 0.0f, 1.0f;
  l.biases << 0.0f, 0.0f;

  l.update(&input);

  CHECK(l.activations(0) == 1.0f);
  CHECK(l.activations(1) == 0.0f);
}

TEST_CASE("Test layer update applies softmax to an output layer") {
  auto input{make_input_layer(vec({1.0f, 1.0f}))};

  models::layer l{2, 2};
  l.is_output = true;
  l.weights << 0.0f, 1.0f, 2.0f, 0.0f;
  l.biases << 0.0f, 0.0f;

  l.update(&input);

  CHECK_THAT(l.activations(0), test_helpers::within_abs(0.268941f));
  CHECK_THAT(l.activations(1), test_helpers::within_abs(0.731059f));
  CHECK_THAT(l.activations.sum(), test_helpers::within_abs(1.0f));
}

TEST_CASE("Test layer update softmax is numerically stable") {
  auto input{make_input_layer(vec({1.0f, 1.0f}))};

  models::layer l{2, 2};
  l.is_output = true;
  l.weights << 0.0f, 1.0f, 2.0f, 0.0f;
  l.biases << 999.0f, 999.0f;

  l.update(&input);

  REQUIRE(std::isfinite(l.activations(0)));
  REQUIRE(std::isfinite(l.activations(1)));
  CHECK_THAT(l.activations(0), test_helpers::within_abs(0.268941f));
  CHECK_THAT(l.activations(1), test_helpers::within_abs(0.731059f));
}

TEST_CASE("Test layer update softmax of identical inputs is uniform") {
  auto input{make_input_layer(vec({1.0f, 1.0f, 1.0f}))};

  models::layer l{4, 3};
  l.is_output = true;

  l.update(&input);

  for (int i{0}; i < 4; i++) {
    CHECK_THAT(l.activations(i), test_helpers::within_abs(0.25f));
  }
}
