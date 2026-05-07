#include <cstdio>
#include <exception>
#include <print>

#include "application.h"

int main() {
  try {
    avah::application app{};
    app.run();
  } catch (const std::exception &e) {
    std::println(stderr, "Runtime error: {}", e.what());
    return 1;
  }
}
