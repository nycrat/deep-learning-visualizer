#include <cstddef>
#include <cstdio>
#include <exception>
#include <print>

#include <Eigen/Core>

#include "ui/application.h"

int main() {
  try {
    ui::application app{};
    app.run();
  } catch (const std::exception &e) {
    std::println(stderr, "Runtime error: {}", e.what());
    return 1;
  }
}
