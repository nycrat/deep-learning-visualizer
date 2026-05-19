#include "catch_amalgamated.h"
#include "shared/math.h"

namespace {

Catch::Matchers::WithinAbsMatcher within_abs(float target) {
  static constexpr double epsilon = 1e-5;
  return Catch::Matchers::WithinAbs(static_cast<double>(target), epsilon);
}

} // namespace

TEST_CASE("Test sigmoid function") {
  CHECK_THAT(shared::sigmoid(-10000.0f), within_abs(0.0f));
  CHECK_THAT(shared::sigmoid(-1.0f), within_abs(0.268941f));
  CHECK_THAT(shared::sigmoid(0.0f), within_abs(0.5f));
  CHECK_THAT(shared::sigmoid(1.0f), within_abs(0.731059f));
  CHECK_THAT(shared::sigmoid(10000.0f), within_abs(1.0f));
}

TEST_CASE("Test derivative of sigmoid function") {
  CHECK_THAT(shared::d_sigmoid(-10000.0f), within_abs(0.0f));
  CHECK_THAT(shared::d_sigmoid(-1.0f), within_abs(0.196612f));
  CHECK_THAT(shared::d_sigmoid(0.0f), within_abs(0.25f));
  CHECK_THAT(shared::d_sigmoid(1.0f), within_abs(0.196612f));
  CHECK_THAT(shared::d_sigmoid(10000.0f), within_abs(0.0f));
}
