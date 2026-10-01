#include "catch_amalgamated.h"
#include "shared/layout.h"
#include "test_helpers.h"

using test_helpers::within_abs;

TEST_CASE("Test layout window aspect is the square grid plus the diagram") {
  CHECK_THAT(shared::WINDOW_ASPECT, within_abs(2.0f));
}

TEST_CASE("Test layout window height fits the whole grid at cell scale") {
  CHECK_THAT(static_cast<float>(shared::CELL_PIXELS), within_abs(30.0f));
  CHECK(shared::WINDOW_HEIGHT ==
        static_cast<int>(shared::GRID_SIZE) * shared::CELL_PIXELS);
}

TEST_CASE("Test layout window width is the height scaled by the aspect") {
  CHECK(shared::WINDOW_WIDTH ==
        static_cast<int>(static_cast<float>(shared::WINDOW_HEIGHT) *
                         shared::WINDOW_ASPECT));
}

TEST_CASE("Test layout window aspect survives the round trip to pixels") {
  // The window sizes itself from these constants, so a width that rounded would
  // make the real aspect disagree with the one every NDC calculation assumes.
  CHECK_THAT(static_cast<float>(shared::WINDOW_WIDTH) /
                 static_cast<float>(shared::WINDOW_HEIGHT),
             within_abs(shared::WINDOW_ASPECT));
}

TEST_CASE("Test layout grid is over the left of the window") {
  CHECK_THAT(shared::GRID_LEFT, within_abs(-1.0f));
  CHECK_THAT(shared::GRID_RIGHT, within_abs(0.0f));
  CHECK(shared::GRID_RIGHT < shared::NETWORK_LEFT);
}

TEST_CASE("Test layout grid is square in pixels") {
  // The grid spans twice as much NDC vertically as horizontally, which is only
  // square on screen because the window is twice as wide as it is tall.
  const auto ndc_width{shared::GRID_RIGHT - shared::GRID_LEFT};
  const auto ndc_height{shared::GRID_TOP - shared::GRID_BOTTOM};

  CHECK_THAT(ndc_height, within_abs(ndc_width * shared::WINDOW_ASPECT));
}

TEST_CASE("Test layout is over grid is true inside and false past the edge") {
  CHECK(shared::is_over_grid(shared::GRID_LEFT));
  CHECK(shared::is_over_grid(0.0f));
  CHECK(shared::is_over_grid(-0.5f));
  CHECK_FALSE(shared::is_over_grid(shared::NETWORK_LEFT));
  CHECK_FALSE(shared::is_over_grid(shared::NETWORK_RIGHT));
}

TEST_CASE("Test layout cursor to cell maps the corners of the grid") {
  // The cursor arrives in NDC with y up, so the bottom left corner is the
  // origin and the top right corner is the far edge of the last cell.
  const auto bottom_left{
      shared::cursor_to_cell({shared::GRID_LEFT, shared::GRID_BOTTOM}, 0.0f)};
  CHECK_THAT(bottom_left.x(), within_abs(0.0f));
  CHECK_THAT(bottom_left.y(), within_abs(0.0f));

  const auto top_right{
      shared::cursor_to_cell({shared::GRID_RIGHT, shared::GRID_TOP}, 0.0f)};
  CHECK_THAT(top_right.x(), within_abs(static_cast<float>(shared::GRID_SIZE)));
  CHECK_THAT(top_right.y(), within_abs(static_cast<float>(shared::GRID_SIZE)));
}

TEST_CASE("Test layout cursor to cell offset shifts by whole cells") {
  const auto center{shared::cursor_to_cell({0.0f, 0.0f}, 0.0f)};
  const auto shifted{shared::cursor_to_cell({0.0f, 0.0f}, 1.5f)};

  CHECK_THAT(center.x() - shifted.x(), within_abs(1.5f));
  CHECK_THAT(center.y() - shifted.y(), within_abs(1.5f));
}

TEST_CASE("Test layout cursor to cell agrees with the grid it paints") {
  // Sampling the middle of each cell must land back on that cell, otherwise
  // the grid would be drawn somewhere other than where painting occurs.
  for (int x{0}; x < shared::GRID_SIZE; x++) {
    for (int y{0}; y < shared::GRID_SIZE; y++) {
      const auto center_x{shared::GRID_LEFT +
                          (static_cast<float>(x) + 0.5f) /
                              static_cast<float>(shared::GRID_SIZE) *
                              (shared::GRID_RIGHT - shared::GRID_LEFT)};
      const auto center_y{shared::GRID_BOTTOM +
                          (static_cast<float>(y) + 0.5f) /
                              static_cast<float>(shared::GRID_SIZE) *
                              (shared::GRID_TOP - shared::GRID_BOTTOM)};

      const auto cell{shared::cursor_to_cell({center_x, center_y}, 0.5f) +
                      Eigen::Vector2f{0.5f, 0.5f}};

      CHECK(static_cast<int>(cell.x()) == x);
      CHECK(static_cast<int>(cell.y()) == y);
    }
  }
}
