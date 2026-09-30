#include "catch_amalgamated.h"
#include "engine/input/constants.h"
#include "engine/input/event_bus.h"
#include "test_helpers.h"

#include <Eigen/Core>

#include <string>
#include <utility>
#include <vector>

namespace {

using test_helpers::within_abs;

} // namespace

TEST_CASE("Test event bus without subscribers does nothing") {
  engine::input::event_bus bus{};

  CHECK_NOTHROW(bus.emit(engine::input::key::a, engine::input::action::down));
  CHECK_NOTHROW(
      bus.emit(engine::input::mouse::button_left, engine::input::action::down));
  CHECK_NOTHROW(bus.emit(Eigen::Vector2f{0.0f, 0.0f}));
}

TEST_CASE("Test event bus notifies every key subscriber") {
  engine::input::event_bus bus{};
  std::vector<std::pair<engine::input::key, engine::input::action>> received;

  bus.subscribe([&](engine::input::key key, engine::input::action action) {
    received.emplace_back(key, action);
  });
  bus.subscribe([&](engine::input::key key, engine::input::action action) {
    received.emplace_back(key, action);
  });

  bus.emit(engine::input::key::space, engine::input::action::down);

  REQUIRE(received.size() == 2);
  CHECK(received.at(0).first == engine::input::key::space);
  CHECK(received.at(0).second == engine::input::action::down);
  CHECK(received.at(1) == received.at(0));
}

TEST_CASE("Test event bus notifies every mouse subscriber") {
  engine::input::event_bus bus{};
  std::vector<engine::input::mouse> buttons;
  std::vector<engine::input::action> actions;

  bus.subscribe([&](engine::input::mouse mouse, engine::input::action action) {
    buttons.push_back(mouse);
    actions.push_back(action);
  });
  bus.subscribe([&](engine::input::mouse, engine::input::action) {
    buttons.push_back(engine::input::mouse::button_right);
    actions.push_back(engine::input::action::up);
  });

  bus.emit(engine::input::mouse::button_left, engine::input::action::down);

  REQUIRE(buttons.size() == 2);
  CHECK(buttons.at(0) == engine::input::mouse::button_left);
  CHECK(actions.at(0) == engine::input::action::down);
  CHECK(buttons.at(1) == engine::input::mouse::button_right);
  CHECK(actions.at(1) == engine::input::action::up);
}

TEST_CASE("Test event bus notifies every cursor subscriber") {
  engine::input::event_bus bus{};
  std::vector<Eigen::Vector2f> positions;

  bus.subscribe(
      [&](const Eigen::Vector2f &position) { positions.push_back(position); });
  bus.subscribe([&](const Eigen::Vector2f &position) {
    positions.push_back(position * 2.0f);
  });

  bus.emit(Eigen::Vector2f{0.25f, -0.5f});

  REQUIRE(positions.size() == 2);
  CHECK_THAT(positions.at(0).x(), within_abs(0.25f));
  CHECK_THAT(positions.at(0).y(), within_abs(-0.5f));
  CHECK_THAT(positions.at(1).x(), within_abs(0.5f));
  CHECK_THAT(positions.at(1).y(), within_abs(-1.0f));
}

TEST_CASE("Test event bus only notifies the matching subscribers") {
  engine::input::event_bus bus{};
  int key_events{};
  int mouse_events{};
  int cursor_events{};

  bus.subscribe(
      [&](engine::input::key, engine::input::action) { key_events++; });
  bus.subscribe(
      [&](engine::input::mouse, engine::input::action) { mouse_events++; });
  bus.subscribe([&](const Eigen::Vector2f &) { cursor_events++; });

  bus.emit(engine::input::key::a, engine::input::action::down);
  bus.emit(engine::input::mouse::button_middle, engine::input::action::up);
  bus.emit(Eigen::Vector2f{1.0f, 1.0f});

  CHECK(key_events == 1);
  CHECK(mouse_events == 1);
  CHECK(cursor_events == 1);
}

TEST_CASE("Test event bus notifies subscribers in subscription order") {
  engine::input::event_bus bus{};
  std::string order;

  bus.subscribe(
      [&](engine::input::key, engine::input::action) { order += "1"; });
  bus.subscribe(
      [&](engine::input::key, engine::input::action) { order += "2"; });
  bus.subscribe(
      [&](engine::input::key, engine::input::action) { order += "3"; });

  bus.emit(engine::input::key::b, engine::input::action::repeat);

  CHECK(order == "123");
}

TEST_CASE("Test event bus emits every action for a key") {
  engine::input::event_bus bus{};
  std::vector<engine::input::action> actions;

  bus.subscribe([&](engine::input::key, engine::input::action action) {
    actions.push_back(action);
  });

  bus.emit(engine::input::key::a, engine::input::action::down);
  bus.emit(engine::input::key::a, engine::input::action::up);
  bus.emit(engine::input::key::a, engine::input::action::repeat);

  REQUIRE(actions.size() == 3);
  CHECK(actions.at(0) == engine::input::action::down);
  CHECK(actions.at(1) == engine::input::action::up);
  CHECK(actions.at(2) == engine::input::action::repeat);
}
