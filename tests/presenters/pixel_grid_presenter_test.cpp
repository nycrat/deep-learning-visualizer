#include "catch_amalgamated.h"
#include "engine/input/constants.h"
#include "engine/input/event_bus.h"
#include "models/mnist.h"
#include "presenters/pixel_grid_presenter.h"
#include "shared/constants.h"
#include "test_helpers.h"
#include "views/pixel_grid_view.h"

#include <Eigen/Core>

#include <algorithm>
#include <array>
#include <memory>

namespace {

using test_helpers::within_abs;

using image_array = std::array<float, shared::TOTAL_PIXELS>;

/**
 * The value of the brush underneath the cursor and the value of a pixel one
 * step away from it, when the cursor is centered on the grid.
 */
constexpr float BRUSH_CENTER{0.934031f};
constexpr float BRUSH_EDGE{0.262443f};

bool is_blank_image(const image_array &drawn) {
  return std::ranges::all_of(drawn, [](float pixel) { return pixel == 0.0f; });
}

bool is_equal_image(const image_array &left, const image_array &right) {
  return left == right;
}

int count_drawn_pixels(const image_array &drawn) {
  return static_cast<int>(
      std::ranges::count_if(drawn, [](float pixel) { return pixel > 0.0f; }));
}

struct grid_recorder {
  image_array drawn{};
  int draw_count{};

  [[nodiscard]] int non_zero_count() const {
    return count_drawn_pixels(drawn);
  }
};

class recording_view : public views::pixel_grid_view {
public:
  explicit recording_view(grid_recorder &recorder) : recorder_{recorder} {
  }

  void draw_grid(const image_array &image_data) override {
    recorder_.drawn = image_data;
    recorder_.draw_count++;
  }

private:
  grid_recorder &recorder_;
};

struct presenter_fixture {
  grid_recorder recorder{};
  engine::input::event_bus bus{};
  presenters::pixel_grid_presenter presenter{
      std::make_unique<models::mnist>(),
      std::make_unique<recording_view>(recorder), bus};

  void click_at(const Eigen::Vector2f &position) {
    presenter.handle_mouse_event(engine::input::mouse::button_left,
                                 engine::input::action::down);
    presenter.handle_cursor_event(position);
  }

  [[nodiscard]] const image_array &image() {
    presenter.render();
    return recorder.drawn;
  }
};

const Eigen::Vector2f CENTER{0.0f, 0.0f};

} // namespace

TEST_CASE("Test pixel grid presenter starts with a blank image") {
  presenter_fixture fixture{};

  CHECK(fixture.image() == image_array{});
  CHECK(fixture.recorder.draw_count == 1);
}

TEST_CASE("Test pixel grid presenter render forwards the image to the view") {
  presenter_fixture fixture{};

  fixture.click_at(CENTER);
  const auto drawn{fixture.image()};

  fixture.presenter.render();
  const auto &redrawn{fixture.recorder.drawn};

  CHECK(is_equal_image(redrawn, drawn));
  CHECK(fixture.recorder.draw_count == 2);
}

TEST_CASE("Test pixel grid presenter ignores the cursor without a click") {
  presenter_fixture fixture{};

  fixture.presenter.handle_cursor_event(CENTER);

  CHECK(fixture.image() == image_array{});
}

TEST_CASE("Test pixel grid presenter draws where the cursor is") {
  presenter_fixture fixture{};

  fixture.click_at(CENTER);

  const auto &drawn{fixture.image()};

  CHECK(fixture.recorder.non_zero_count() == 12);
  CHECK_THAT(drawn.at(13 + 13 * shared::GRID_SIZE), within_abs(BRUSH_CENTER));
  CHECK_THAT(drawn.at(14 + 13 * shared::GRID_SIZE), within_abs(BRUSH_CENTER));
  CHECK_THAT(drawn.at(13 + 14 * shared::GRID_SIZE), within_abs(BRUSH_CENTER));
  CHECK_THAT(drawn.at(14 + 14 * shared::GRID_SIZE), within_abs(BRUSH_CENTER));
  CHECK_THAT(drawn.at(12 + 13 * shared::GRID_SIZE), within_abs(BRUSH_EDGE));
  CHECK_THAT(drawn.at(15 + 14 * shared::GRID_SIZE), within_abs(BRUSH_EDGE));
  CHECK(drawn.at(0) == 0.0f);
  CHECK(drawn.at(shared::TOTAL_PIXELS - 1) == 0.0f);
}

TEST_CASE(
    "Test pixel grid presenter stops drawing when the button is released") {
  presenter_fixture fixture{};

  fixture.click_at(CENTER);
  const auto drawn{fixture.image()};

  fixture.presenter.handle_mouse_event(engine::input::mouse::button_left,
                                       engine::input::action::up);
  fixture.presenter.handle_cursor_event(Eigen::Vector2f{0.5f, 0.5f});

  CHECK(is_equal_image(fixture.image(), drawn));
}

TEST_CASE("Test pixel grid presenter ignores the other mouse buttons") {
  presenter_fixture fixture{};

  fixture.presenter.handle_mouse_event(engine::input::mouse::button_right,
                                       engine::input::action::down);
  fixture.presenter.handle_cursor_event(CENTER);

  CHECK(fixture.image() == image_array{});
}

TEST_CASE("Test pixel grid presenter keeps the brightest pixel") {
  presenter_fixture fixture{};

  fixture.click_at(CENTER);
  const auto first{fixture.image()};
  fixture.click_at(Eigen::Vector2f{0.6f, 0.0f});
  const auto &second{fixture.image()};

  CHECK(count_drawn_pixels(second) > count_drawn_pixels(first));
  CHECK_THAT(second.at(13 + 13 * shared::GRID_SIZE),
             within_abs(first.at(13 + 13 * shared::GRID_SIZE)));
  CHECK(second.at(13 + 13 * shared::GRID_SIZE) > 0.0f);
}

TEST_CASE("Test pixel grid presenter clips the brush to the grid") {
  presenter_fixture fixture{};

  fixture.click_at(Eigen::Vector2f{-1.0f, -1.0f});

  const auto &drawn{fixture.image()};

  CHECK(fixture.recorder.non_zero_count() == 3);
  CHECK_THAT(drawn.at(0), within_abs(BRUSH_CENTER));
  CHECK_THAT(drawn.at(1), within_abs(BRUSH_EDGE));
  CHECK(drawn.at(shared::TOTAL_PIXELS - 1) == 0.0f);
}

TEST_CASE("Test pixel grid presenter clears the image on r") {
  presenter_fixture fixture{};

  fixture.click_at(CENTER);
  REQUIRE(count_drawn_pixels(fixture.image()) > 0);

  fixture.presenter.handle_key_event(engine::input::key::r,
                                     engine::input::action::down);

  CHECK(fixture.image() == image_array{});
}

TEST_CASE("Test pixel grid presenter ignores key releases") {
  presenter_fixture fixture{};

  fixture.click_at(CENTER);
  const auto drawn{fixture.image()};

  fixture.presenter.handle_key_event(engine::input::key::r,
                                     engine::input::action::up);

  CHECK(is_equal_image(fixture.image(), drawn));
}

TEST_CASE("Test pixel grid presenter predicts the drawn image on space") {
  presenter_fixture fixture{};

  fixture.click_at(CENTER);

  CHECK_NOTHROW(fixture.presenter.handle_key_event(
      engine::input::key::space, engine::input::action::down));
  CHECK(count_drawn_pixels(fixture.image()) == 12);
}

TEST_CASE("Test pixel grid presenter draws the events sent over the bus") {
  presenter_fixture fixture{};

  fixture.bus.emit(engine::input::mouse::button_left,
                   engine::input::action::down);
  fixture.bus.emit(CENTER);

  const auto &drawn{fixture.image()};

  CHECK(count_drawn_pixels(drawn) == 12);
  CHECK(drawn.at(13 + 13 * shared::GRID_SIZE) > 0.0f);
}

TEST_CASE(
    "Test pixel grid presenter clears the image with a key from the bus") {
  presenter_fixture fixture{};

  fixture.click_at(CENTER);
  REQUIRE(count_drawn_pixels(fixture.image()) > 0);

  fixture.bus.emit(engine::input::key::r, engine::input::action::down);

  CHECK(is_blank_image(fixture.image()));
}
