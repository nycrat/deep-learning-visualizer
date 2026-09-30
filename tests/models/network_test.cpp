#include "catch_amalgamated.h"
#include "models/data_point.h"
#include "models/network.h"
#include "test_helpers.h"

#include <Eigen/Core>

#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

using test_helpers::temp_file;
using test_helpers::within_abs;

/**
 * Exposes the protected members of the network that the tests need to reach.
 */
class test_network : public models::network {
public:
  explicit test_network(const std::vector<int> &layer_sizes)
      : models::network{layer_sizes} {
  }

  explicit test_network(const std::filesystem::path &file_path)
      : models::network{file_path} {
  }

  void forward(const Eigen::VectorXf &input) {
    set_input(input);
    update();
  }

  [[nodiscard]] const Eigen::VectorXf &activations() const {
    return output();
  }

  [[nodiscard]] float train(const std::vector<models::data_point> &batch) {
    return backpropagate(batch);
  }

  void initialize() {
    initialize_weights();
  }
};

/**
 * A network with one hidden layer which computes softmax([3, 1]) for the input
 * [1, 2, 3]. The hidden layer copies the first and the last input, and the
 * output layer swaps them.
 */
constexpr std::string_view SMALL_NETWORK{"3\n"
                                         "3 2 2 \n"
                                         "\n"
                                         "0 0 \n"
                                         "0 0 \n"
                                         "\n"
                                         "1 0 0 \n"
                                         "0 0 1 \n"
                                         "\n"
                                         "0 1 \n"
                                         "1 0 \n"};

/**
 * A network with all zero weights and biases, so that every input is mapped to
 * a uniform output.
 */
constexpr std::string_view ZERO_NETWORK{"3\n"
                                        "3 2 2 \n"
                                        "\n"
                                        "0 0 \n"
                                        "0 0 \n"
                                        "\n"
                                        "0 0 0 \n"
                                        "0 0 0 \n"
                                        "\n"
                                        "0 0 0 \n"
                                        "0 0 0 \n"};

/**
 * A network whose hidden layer holds x, 1 - x for both inputs plus a constant,
 * so that the output layer is a linear model over an expressive basis. The
 * output layer only looks at the first hidden unit, which makes the initial
 * prediction depend on the first input alone.
 */
constexpr std::string_view XOR_NETWORK{"3\n"
                                       "2 5 2 \n"
                                       "\n"
                                       "0 0 0 0 1 \n"
                                       "0.1 0.2 \n"
                                       "\n"
                                       "1 0 \n"
                                       "-1 0 \n"
                                       "0 1 \n"
                                       "0 -1 \n"
                                       "0 0 \n"
                                       "\n"
                                       "0.5 0 0 0 0 \n"
                                       "0 0.5 0 0 0 \n"};

Eigen::VectorXf vec(std::initializer_list<float> values) {
  return Eigen::VectorXf{values};
}

std::string read_file(const std::filesystem::path &path) {
  std::ifstream file{path};
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::vector<std::string> tokenize(const std::string &text) {
  std::istringstream stream{text};
  std::vector<std::string> tokens;
  for (std::string token; stream >> token;) {
    tokens.push_back(std::move(token));
  }
  return tokens;
}

Eigen::VectorXf run_forward(test_network &net, const Eigen::VectorXf &input) {
  net.forward(input);
  return net.activations();
}

bool all_positive(const Eigen::VectorXf &v) {
  return (v.array() > 0.0f).all();
}

int argmax(const Eigen::VectorXf &v) {
  int index{0};
  for (Eigen::Index i{1}; i < v.size(); i++) {
    if (v(i) > v(index)) {
      index = static_cast<int>(i);
    }
  }
  return index;
}

template <std::size_t N>
Eigen::VectorXf to_vector(const std::array<float, N> &values) {
  return Eigen::Map<const Eigen::VectorXf>{values.data(),
                                           static_cast<Eigen::Index>(N)};
}

/**
 * An owned training sample, so that the views handed to the network stay valid
 * for as long as the sample is alive.
 */
template <std::size_t N> struct sample {
  std::array<float, N> input{};
  std::array<float, 2> output{};

  [[nodiscard]] Eigen::VectorXf input_vector() const {
    return to_vector(input);
  }

  [[nodiscard]] Eigen::VectorXf output_vector() const {
    return to_vector(output);
  }

  [[nodiscard]] models::data_point data_point() const {
    return {Eigen::Map<const Eigen::VectorXf>{input.data(),
                                              static_cast<Eigen::Index>(N)},
            Eigen::Map<const Eigen::VectorXf>{output.data(), 2}};
  }
};

template <std::size_t N>
std::vector<models::data_point>
to_batch(const std::vector<sample<N>> &samples) {
  std::vector<models::data_point> batch;
  batch.reserve(samples.size());
  for (const auto &s : samples) {
    batch.push_back(s.data_point());
  }
  return batch;
}

std::vector<sample<2>> xor_samples() {
  return {
      {{0.0f, 0.0f}, {1.0f, 0.0f}},
      {{0.0f, 1.0f}, {0.0f, 1.0f}},
      {{1.0f, 0.0f}, {0.0f, 1.0f}},
      {{1.0f, 1.0f}, {1.0f, 0.0f}},
  };
}

} // namespace

TEST_CASE("Test network constructor throws for a single layer") {
  CHECK_THROWS_AS(test_network(std::vector<int>{4}), std::runtime_error);
}

TEST_CASE("Test network constructor throws without layers") {
  CHECK_THROWS_AS((test_network{std::vector<int>{}}), std::runtime_error);
}

TEST_CASE("Test network constructor accepts two layers") {
  CHECK_NOTHROW((test_network{std::vector<int>{4, 2}}));
}

TEST_CASE("Test network constructor initializes a forward pass") {
  const temp_file file{"network_forward"};
  file.write_text(SMALL_NETWORK);

  test_network net{file.path()};

  const auto result{run_forward(net, vec({1.0f, 2.0f, 3.0f}))};

  REQUIRE(result.size() == 2);
  CHECK_THAT(result(0), within_abs(0.880797f));
  CHECK_THAT(result(1), within_abs(0.119203f));
}

TEST_CASE("Test network forward pass is a probability distribution") {
  const temp_file file{"network_distribution"};
  file.write_text(SMALL_NETWORK);

  test_network net{file.path()};

  const auto result{run_forward(net, vec({5.0f, 0.0f, 1.0f}))};

  CHECK(all_positive(result));
  CHECK_THAT(result.sum(), within_abs(1.0f));
}

TEST_CASE("Test network forward pass is deterministic") {
  const temp_file file{"network_deterministic"};
  file.write_text(SMALL_NETWORK);

  test_network net{file.path()};

  const auto first{run_forward(net, vec({1.0f, 2.0f, 3.0f}))};
  const auto second{run_forward(net, vec({1.0f, 2.0f, 3.0f}))};

  CHECK(first.isApprox(second));
}

TEST_CASE("Test network without weights maps every input to a uniform output") {
  const temp_file file{"network_uniform"};
  file.write_text(ZERO_NETWORK);

  test_network net{file.path()};

  for (int i{0}; i < 4; i++) {
    const auto result{
        run_forward(net, vec({static_cast<float>(i), 0.0f, 1.0f}))};
    CHECK_THAT(result(0), within_abs(0.5f));
    CHECK_THAT(result(1), within_abs(0.5f));
  }
}

TEST_CASE(
    "Test network from file constructor throws when the file is missing") {
  CHECK_THROWS_AS((test_network{std::filesystem::path{"no_such_model.mlp"}}),
                  std::runtime_error);
}

TEST_CASE("Test network to file writes the header of the network") {
  const temp_file in_file{"network_to_file_in"};
  const temp_file out_file{"network_to_file_out"};
  in_file.write_text(SMALL_NETWORK);

  test_network net{in_file.path()};
  net.to_file(out_file.path());

  const auto tokens{tokenize(read_file(out_file.path()))};

  REQUIRE(tokens.size() == 1 + 3 + 4 + 10);
  CHECK(tokens.at(0) == "3");
  CHECK(tokens.at(1) == "3");
  CHECK(tokens.at(2) == "2");
  CHECK(tokens.at(3) == "2");
}

TEST_CASE("Test network to file writes the biases and the weights") {
  const temp_file in_file{"network_to_file_values_in"};
  const temp_file out_file{"network_to_file_values_out"};
  in_file.write_text(SMALL_NETWORK);

  test_network net{in_file.path()};
  net.to_file(out_file.path());

  const std::vector<std::string> expected{"0", "0", "0", "0", "1", "0", "0",
                                          "0", "0", "1", "0", "1", "1", "0"};
  const auto tokens{tokenize(read_file(out_file.path()))};

  REQUIRE(tokens.size() > expected.size());
  CHECK((std::vector<std::string>{tokens.end() - expected.size(),
                                  tokens.end()}) == expected);
}

TEST_CASE("Test network to file round trips through the file constructor") {
  const temp_file in_file{"network_round_trip_in"};
  const temp_file out_file{"network_round_trip_out"};
  in_file.write_text(SMALL_NETWORK);

  test_network net{in_file.path()};
  net.to_file(out_file.path());

  test_network restored{out_file.path()};

  const auto expected{run_forward(net, vec({1.0f, 2.0f, 3.0f}))};
  const auto actual{run_forward(restored, vec({1.0f, 2.0f, 3.0f}))};

  CHECK_THAT(actual(0), within_abs(expected(0)));
  CHECK_THAT(actual(1), within_abs(expected(1)));
}

TEST_CASE("Test network round trip keeps the predictions stable") {
  const temp_file in_file{"network_round_trip_predictions_in"};
  const temp_file out_file{"network_round_trip_predictions_out"};
  in_file.write_text(SMALL_NETWORK);

  test_network net{in_file.path()};
  net.to_file(out_file.path());

  test_network restored{out_file.path()};

  for (int i{0}; i < 8; i++) {
    const auto input{vec({static_cast<float>(i), 1.0f, 2.0f})};
    CHECK(
        run_forward(net, input).isApprox(run_forward(restored, input), 1e-4f));
  }
}

TEST_CASE("Test network backpropagate returns the mean cross entropy") {
  const temp_file file{"network_cost"};
  file.write_text(SMALL_NETWORK);

  test_network net{file.path()};

  const std::vector<sample<3>> samples{{{1.0f, 2.0f, 3.0f}, {1.0f, 3.0f}},
                                       {{0.0f, 1.0f, 3.0f}, {2.0f, 5.0f}}};

  float expected{};
  for (const auto &s : samples) {
    const auto prediction{run_forward(net, s.input_vector())};
    expected +=
        -std::log(static_cast<float>(prediction.dot(s.output_vector())));
  }
  expected /= static_cast<float>(samples.size());

  CHECK_THAT(net.train(to_batch(samples)), within_abs(expected));
}

TEST_CASE("Test network backpropagate averages the cost of the batch") {
  const temp_file file{"network_cost_mean"};
  file.write_text(XOR_NETWORK);

  test_network net{file.path()};

  const auto samples{xor_samples()};

  const auto individual = [&net, &samples] {
    float total{};
    for (const auto &s : samples) {
      const auto prediction{run_forward(net, s.input_vector())};
      total += -std::log(static_cast<float>(prediction.dot(s.output_vector())));
    }
    return total / static_cast<float>(samples.size());
  }();

  CHECK_THAT(net.train(to_batch(samples)), within_abs(individual));
}

TEST_CASE("Test network backpropagate reduces the cost of the batch") {
  const temp_file file{"network_reduces_cost"};
  file.write_text(XOR_NETWORK);

  const auto samples{xor_samples()};
  const auto batch{to_batch(samples)};

  test_network net{file.path()};

  const auto initial_cost{net.train(batch)};
  float cost{};

  for (int i{0}; i < 1000; i++) {
    cost = net.train(batch);
  }

  CHECK(cost < initial_cost);
  CHECK(cost < 0.1f);
}

TEST_CASE("Test network learns xor") {
  const temp_file file{"network_xor"};
  file.write_text(XOR_NETWORK);

  const auto samples{xor_samples()};
  const auto batch{to_batch(samples)};

  test_network net{file.path()};

  for (int i{0}; i < 2000; i++) {
    static_cast<void>(net.train(batch));
  }

  for (const auto &s : samples) {
    const auto prediction{run_forward(net, s.input_vector())};
    CHECK(argmax(prediction) == argmax(s.output_vector()));
  }
}

TEST_CASE("Test network initialize weights produces a non uniform network") {
  test_network net{std::vector<int>{3, 8, 2}};

  const auto before{run_forward(net, vec({1.0f, 2.0f, 3.0f}))};
  CHECK_THAT(before(0), within_abs(0.5f));

  net.initialize();

  const auto after{run_forward(net, vec({1.0f, 2.0f, 3.0f}))};

  CHECK_FALSE(after.isApprox(before));
  CHECK(all_positive(after));
  CHECK_THAT(after.sum(), within_abs(1.0f));
}

TEST_CASE("Test network initialize weights differs between networks") {
  test_network first{std::vector<int>{4, 4, 2}};
  test_network second{std::vector<int>{4, 4, 2}};

  first.initialize();
  second.initialize();

  const auto input{vec({1.0f, 0.0f, 1.0f, 0.0f})};

  CHECK_FALSE(run_forward(first, input).isApprox(run_forward(second, input)));
}
