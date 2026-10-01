#pragma once

/**
 * @file
 * @brief Where the pixel grid and the network sit inside the window.
 *
 * The window is twice as wide as it is tall, which leaves a square region on
 * the left for the paintable grid and a square region on the right for the
 * network visualization. Every bound is in normalized device coordinates, where
 * the window spans [-1, 1] on each axis.
 */

#include "shared/constants.h"

#include <Eigen/Core>

namespace shared {

/**
 * The NDC bounds of the paintable pixel grid.
 *
 * The grid is a square in pixels even though these bounds are not square in
 * NDC, because the window is twice as wide as it is tall.
 */
inline constexpr float GRID_LEFT{-1.0f};
inline constexpr float GRID_RIGHT{0.0f};
inline constexpr float GRID_BOTTOM{-1.0f};
inline constexpr float GRID_TOP{1.0f};

/** The NDC bounds of the network diagram. */
inline constexpr float NETWORK_LEFT{0.05f};
inline constexpr float NETWORK_RIGHT{0.95f};
inline constexpr float NETWORK_BOTTOM{-0.95f};
inline constexpr float NETWORK_TOP{0.95f};

/**
 * How many times wider the window is than it is tall.
 *
 * NDC spans [-1, 1] on both axes regardless of the window's shape, so one NDC
 * unit covers twice as many pixels horizontally as it does vertically. Anything
 * meant to look square has to be stretched by this factor along y.
 */
inline constexpr float WINDOW_ASPECT{2.0f};

/** How many device pixels one cell of the grid is drawn at. */
inline constexpr int CELL_PIXELS{30};

/** The height of the window in pixels, which fixes the scale of the grid. */
inline constexpr int WINDOW_HEIGHT{GRID_SIZE * CELL_PIXELS};

/** The width of the window in pixels, derived from the aspect ratio. */
inline constexpr int WINDOW_WIDTH{
    static_cast<int>(static_cast<float>(WINDOW_HEIGHT) * WINDOW_ASPECT)};

/**
 * The aspect ratio has to divide the height exactly.
 *
 * Otherwise the width would round and the window's true aspect would no longer
 * match WINDOW_ASPECT, which would quietly stretch everything drawn in NDC.
 */
static_assert(static_cast<float>(WINDOW_WIDTH) ==
                  static_cast<float>(WINDOW_HEIGHT) * WINDOW_ASPECT,
              "WINDOW_ASPECT must divide WINDOW_HEIGHT exactly");

/**
 * The share of the diagram's width given to the input layer.
 *
 * The input is drawn as a 28 by 28 image, so it needs to be wide enough to
 * read as a digit, while the hidden layers only need room for a column of
 * dots.
 */
inline constexpr float INPUT_COLUMN_SHARE{0.32f};

/**
 * Whether a cursor at this NDC position is over the paintable grid.
 *
 * @param cursor_x The NDC x position of the cursor.
 */
inline bool is_over_grid(float cursor_x) {
  return cursor_x <= GRID_RIGHT;
}

/**
 * Maps an NDC position onto the grid cell underneath it.
 *
 * @param position The NDC position of the cursor.
 * @param offset A brush offset subtracted from the cell coordinates.
 * @return The fractional cell coordinates, with y measured from the bottom.
 */
inline Eigen::Vector2f cursor_to_cell(const Eigen::Vector2f &position,
                                      float offset) {
  const auto cell_x{(position.x() - GRID_LEFT) / (GRID_RIGHT - GRID_LEFT) *
                    static_cast<float>(GRID_SIZE)};
  const auto cell_y{(position.y() - GRID_BOTTOM) / (GRID_TOP - GRID_BOTTOM) *
                    static_cast<float>(GRID_SIZE)};

  return {cell_x - offset, cell_y - offset};
}

} // namespace shared
