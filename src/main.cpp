#include <cstddef>
#include <cstdio>
#include <exception>
#include <print>
#include <vector>

#include <Eigen/Core>

#include "mlp/network.h"
#include "ui/application.h"

namespace {

// TODO remove later
void test_mlp() {
  std::vector<std::size_t> config{1, 3, 3, 10, 2};
  mlp::network network{config};
  network.initialize_weights();

  Eigen::VectorXf input{1};
  input << 1.0f;

  Eigen::VectorXf output{2};
  output << 0.2f, 0.8f;

  std::vector<Eigen::VectorXf> inputs{};
  std::vector<Eigen::VectorXf> outputs{};

  for (int i{0}; i < 100; i++) {
    inputs.push_back(input);
    outputs.push_back(output);
  }

  for (int i{0}; i < 6000; i++) {
    network.backpropagate(inputs, outputs);
    std::println("{}", network.output());
  }
}

} // namespace

int main() {
  test_mlp();
  try {
    ui::application app{};
    app.run();
  } catch (const std::exception &e) {
    std::println(stderr, "Runtime error: {}", e.what());
    return 1;
  }
}
