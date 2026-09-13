#include <cstdio>
#include <exception>
#include <memory>
#include <print>

#include "engine/application.h"
#include "scenes/example_scene.h"

int main() {
  try {
    engine::application app;
    auto scene = std::make_unique<scenes::example_scene>(app.renderer());
    app.set_scene(std::move(scene));
    app.run();
  } catch (const std::exception &e) {
    std::println(stderr, "Runtime error: {}", e.what());
    return 1;
  }
}
