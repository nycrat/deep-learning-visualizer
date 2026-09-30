#include "catch_amalgamated.h"
#include "models/mnist.h"
#include "shared/constants.h"
#include "test_helpers.h"

#include <Eigen/Core>

#include <cstdint>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

using test_helpers::temp_file;
using test_helpers::within_abs;

/**
 * Exposes the activations of the model, which are only reachable from a
 * subclass of the network.
 */
class test_mnist : public models::mnist {
public:
  using models::mnist::mnist;

  [[nodiscard]] const Eigen::VectorXf &activations() const {
    return output();
  }
};

Eigen::VectorXf blank_input() {
  return Eigen::VectorXf::Zero(shared::TOTAL_PIXELS);
}

Eigen::VectorXf first_pixel_input(float value) {
  Eigen::VectorXf input{blank_input()};
  input(0) = value;
  return input;
}

/**
 * Writes a model whose single active hidden unit relays the first pixel of the
 * input to the output layer. Digit 3 gains from a lit first pixel, while digit
 * 7 wins everything else.
 */
std::string make_relay_model() {
  std::ostringstream file;

  file << "3\n";
  file << shared::TOTAL_PIXELS << " 2 " << shared::TOTAL_DIGITS << " \n";
  file << "\n";

  file << "0 0 \n";
  file << "0 0 0 0 0 0 0 1 0 0 \n";

  file << "1 ";
  for (std::int64_t i{1}; i < shared::TOTAL_PIXELS; i++) {
    file << "0 ";
  }
  file << "\n";

  for (std::int64_t i{0}; i < shared::TOTAL_PIXELS; i++) {
    file << "0 ";
  }
  file << "\n";

  for (std::int64_t digit{0}; digit < shared::TOTAL_DIGITS; digit++) {
    file << (digit == 3 ? "5 " : (digit == 7 ? "-5 " : "0 ")) << "0 \n";
  }

  return file.str();
}

} // namespace

TEST_CASE("Test mnist from file constructor throws when the file is missing") {
  CHECK_THROWS_AS(models::mnist{std::filesystem::path{"no_such_model.mlp"}},
                  std::runtime_error);
}

TEST_CASE("Test mnist predict returns a digit for an untrained model") {
  test_mnist model{};

  for (int i{0}; i < 8; i++) {
    const auto predicted{model.predict(blank_input())};
    CHECK(predicted >= 0);
    CHECK(predicted < shared::TOTAL_DIGITS);
  }
}

TEST_CASE("Test mnist predict output is a probability distribution") {
  test_mnist model{};
  const auto predicted{model.predict(first_pixel_input(1.0f))};

  const auto &activations{model.activations()};

  REQUIRE(activations.size() == shared::TOTAL_DIGITS);
  CHECK((activations.array() > 0.0f).all());
  CHECK_THAT(activations.sum(), within_abs(1.0f));

  int argmax{0};
  for (std::int64_t digit{1}; digit < shared::TOTAL_DIGITS; digit++) {
    if (activations(digit) > activations(argmax)) {
      argmax = static_cast<int>(digit);
    }
  }
  CHECK(argmax == predicted);
}

TEST_CASE("Test mnist predict reads the weights from a file") {
  const temp_file file{"mnist_relay"};
  file.write_text(make_relay_model());

  test_mnist model{file.path()};

  CHECK(model.predict(blank_input()) == 7);
  CHECK(model.predict(first_pixel_input(1.0f)) == 3);
  CHECK(model.predict(first_pixel_input(0.5f)) == 3);
  CHECK(model.predict(first_pixel_input(10.0f)) == 3);
}

TEST_CASE("Test mnist predict breaks ties in favor of the lowest digit") {
  const temp_file file{"mnist_relay_tie"};
  file.write_text(make_relay_model());

  test_mnist model{file.path()};

  // The gains cancel out at 0.1, leaving digits 3 and 7 tied.
  CHECK(model.predict(first_pixel_input(0.1f)) == 3);
}

TEST_CASE("Test mnist predict normalizes the output of a trained model") {
  const temp_file file{"mnist_trained_normalized"};
  file.write_text(make_relay_model());

  test_mnist model{file.path()};
  static_cast<void>(model.predict(first_pixel_input(1.0f)));

  const auto &activations{model.activations()};

  CHECK_THAT(activations.sum(), within_abs(1.0f));
  CHECK(activations(3) > activations(7));
  for (std::int64_t digit{0}; digit < shared::TOTAL_DIGITS; digit++) {
    if (digit != 3) {
      CHECK(activations(3) > activations(digit));
    }
  }
}
