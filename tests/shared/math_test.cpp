#include "catch_amalgamated.h"
#include "shared/math.h"
#include "test_helpers.h"

using test_helpers::within_abs;

TEST_CASE("Test sigmoid function") {
  CHECK_THAT(shared::sigmoid(-10000.0f), within_abs(0.0f));
  CHECK_THAT(shared::sigmoid(-1.0f), within_abs(0.268941f));
  CHECK_THAT(shared::sigmoid(0.0f), within_abs(0.5f));
  CHECK_THAT(shared::sigmoid(1.0f), within_abs(0.731059f));
  CHECK_THAT(shared::sigmoid(10000.0f), within_abs(1.0f));
}

TEST_CASE("Test sigmoid function is bounded by zero and one") {
  for (int i{-15}; i <= 15; i++) {
    const auto value{shared::sigmoid(static_cast<float>(i))};
    CHECK(value > 0.0f);
    CHECK(value < 1.0f);
  }
}

TEST_CASE("Test sigmoid function is monotonically increasing") {
  float previous{shared::sigmoid(-10.0f)};
  for (int i{-9}; i <= 10; i++) {
    const auto current{shared::sigmoid(static_cast<float>(i))};
    CHECK(current > previous);
    previous = current;
  }
}

TEST_CASE("Test derivative of sigmoid function") {
  CHECK_THAT(shared::d_sigmoid(-10000.0f), within_abs(0.0f));
  CHECK_THAT(shared::d_sigmoid(-1.0f), within_abs(0.196612f));
  CHECK_THAT(shared::d_sigmoid(0.0f), within_abs(0.25f));
  CHECK_THAT(shared::d_sigmoid(1.0f), within_abs(0.196612f));
  CHECK_THAT(shared::d_sigmoid(10000.0f), within_abs(0.0f));
}

TEST_CASE("Test derivative of sigmoid function matches a finite difference") {
  constexpr float h{1e-3f};
  for (int i{-4}; i <= 4; i++) {
    const float x{static_cast<float>(i)};
    const auto finite_difference{
        (shared::sigmoid(x + h) - shared::sigmoid(x - h)) / (2 * h)};
    CHECK_THAT(shared::d_sigmoid(x), within_abs(finite_difference, 1e-4));
  }
}

TEST_CASE("Test relu function") {
  CHECK_THAT(shared::relu(-10000.0f), within_abs(0.0f));
  CHECK_THAT(shared::relu(-1.0f), within_abs(0.0f));
  CHECK_THAT(shared::relu(0.0f), within_abs(0.0f));
  CHECK_THAT(shared::relu(1.0f), within_abs(1.0f));
  CHECK_THAT(shared::relu(10000.0f), within_abs(10000.0f, 1e-2));
}

TEST_CASE("Test relu function is non-negative and never decreases") {
  float previous{shared::relu(-10.0f)};
  for (int i{-9}; i <= 10; i++) {
    const auto current{shared::relu(static_cast<float>(i))};
    CHECK(current >= 0.0f);
    CHECK(current >= previous);
    previous = current;
  }
}

TEST_CASE("Test derivative of relu function") {
  CHECK_THAT(shared::d_relu(-10000.0f), within_abs(0.0f));
  CHECK_THAT(shared::d_relu(-1.0f), within_abs(0.0f));
  CHECK_THAT(shared::d_relu(0.0f), within_abs(0.0f));
  CHECK_THAT(shared::d_relu(1.0f), within_abs(1.0f));
  CHECK_THAT(shared::d_relu(10000.0f), within_abs(1.0f));
}
