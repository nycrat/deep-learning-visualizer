#pragma once

/** @file */

#include "engine/graphics/renderer.h"
#include "shared/constants.h"
#include "shared/layout.h"
#include "views/network_view.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <vector>

namespace views {

/**
 * Draws the network as columns of neurons wired together by connections.
 *
 * Neuron brightness follows its activation, so the picture is driven by
 * whatever is currently in the input layer. Connections are colored by the sign
 * of their weight and faded by how much that weight actually contributes, so
 * the bright ones are the ones doing the work for this particular input.
 *
 * The input layer is drawn as the image it is, a grid of lit pixels, since it
 * has no incoming weights and 784 dots in a column would be unreadable.
 *
 * Only the strongest connections into each neuron are drawn. Adjacent layers
 * are fully connected, which for this network is over 130,000 connections, more
 * than the vertex buffer can hold and far more than anyone can read. The
 * truncation is per destination neuron rather than global, so no neuron is left
 * with an empty fan of inputs.
 */
class network_view_gl : public network_view {
public:
  explicit network_view_gl(engine::graphics::renderer &renderer)
      : renderer_(renderer) {
  }

  void draw_network(const std::vector<models::layer> &layers) override {
    if (layers.size() < MIN_LAYERS) {
      return;
    }

    for (std::size_t i{0}; i < layers.size(); i++) {
      if (i > 0) {
        draw_connections(layers.at(i - 1), layers.at(i), i, layers.size());
      }

      if (i == 0) {
        draw_input_layer(layers.front());
      } else {
        draw_neurons(layers.at(i), i, layers.size());
      }
    }

    draw_winner(layers);
  }

private:
  /**
   * Draws the input layer as a grid of lit pixels.
   */
  void draw_input_layer(const models::layer &layer) {
    const auto cell{input_cell()};

    for (Eigen::Index i{0}; i < layer.activations.size(); i++) {
      const auto value{layer.activations(i)};
      if (value <= 0.0f) {
        continue;
      }

      const auto anchor{input_pixel_origin(static_cast<std::size_t>(i), cell)};

      renderer_.get().set_color(Eigen::Vector3f{1.0f, 1.0f, 1.0f} *
                                std::fmin(value, 1.0f));
      renderer_.get().draw_quad(
          anchor, anchor + Eigen::Vector2f{cell, 0.0f},
          anchor + Eigen::Vector2f{cell, cell * shared::WINDOW_ASPECT},
          anchor + Eigen::Vector2f{0.0f, cell * shared::WINDOW_ASPECT});
    }
  }

  /**
   * Draws one layer as a vertical column of circles, one per neuron.
   *
   * Brightness follows the activation, normalized against the largest
   * activation in the same layer, so a column stays legible however large its
   * values happen to grow.
   */
  void draw_neurons(const models::layer &layer, std::size_t column,
                    std::size_t columns) {
    const auto count{static_cast<std::size_t>(layer.activations.size())};
    if (count == 0) {
      return;
    }

    const auto peak{activation_peak(layer)};
    const auto radius{std::fmin(spacing(count) * NEURON_FILL, MAX_RADIUS)};

    for (std::size_t i{0}; i < count; i++) {
      const auto intensity{relative_activation(layer, i, peak)};

      renderer_.get().set_color(NEURON_COLOR * intensity);
      draw_circle(neuron_position(column, i, count, columns), radius);
    }
  }

  /**
   * Draws the strongest connections between two adjacent layers.
   *
   * For each neuron in the destination layer the connections are ranked by
   * |weight| times the activation of the neuron feeding them, and only the top
   * few are drawn. Multiplying by the source activation is what makes the
   * picture track the input: a large weight attached to a dark pixel
   * contributes nothing, and is correctly drawn faintly or not at all.
   */
  void draw_connections(const models::layer &source, const models::layer &dest,
                        std::size_t column, std::size_t columns) {
    const auto source_n{static_cast<std::size_t>(source.activations.size())};
    const auto dest_n{static_cast<std::size_t>(dest.activations.size())};

    if (source_n == 0 || dest_n == 0) {
      return;
    }

    for (std::size_t j{0}; j < dest_n; j++) {
      rank_incoming(source, dest, j);
      if (candidates_.empty()) {
        continue;
      }

      const auto strongest{candidates_.front().strength};
      if (strongest <= 0.0f) {
        continue;
      }

      const auto column_index{static_cast<Eigen::Index>(j)};
      const auto to{neuron_position(column, j, dest_n, columns)};

      for (const auto &candidate : candidates_) {
        // A layer's weights are indexed [destination][source], since
        // z = weights * previous_activations.
        const auto weight{dest.weights(
            column_index, static_cast<Eigen::Index>(candidate.source))};
        const auto share{candidate.strength / strongest};
        const auto hue{weight >= 0.0f ? POSITIVE_COLOR : NEGATIVE_COLOR};

        renderer_.get().set_color(hue * (CONNECTION_GAIN * share));
        renderer_.get().draw_line(
            neuron_position(column - 1, candidate.source, source_n, columns),
            to, CONNECTION_THICKNESS);
      }
    }
  }

  /**
   * Ranks the connections into one destination neuron, strongest first, and
   * leaves the top KEEP_PER_NEURON of them in candidates_.
   *
   * Reuses the scratch vector so that drawing does not allocate per neuron.
   */
  void rank_incoming(const models::layer &source, const models::layer &dest,
                     std::size_t destination) {
    const auto column{static_cast<Eigen::Index>(destination)};
    candidates_.clear();

    for (Eigen::Index i{0}; i < source.activations.size(); i++) {
      const auto strength{std::fabs(dest.weights(column, i)) *
                          std::fabs(source.activations(i))};

      if (strength > 0.0f) {
        candidates_.push_back(
            {.strength = strength, .source = static_cast<std::size_t>(i)});
      }
    }

    const auto keep{std::min(candidates_.size(), KEEP_PER_NEURON)};
    std::partial_sort(candidates_.begin(),
                      candidates_.begin() + static_cast<std::ptrdiff_t>(keep),
                      candidates_.end(),
                      [](const candidate &l, const candidate &r) {
                        return l.strength > r.strength;
                      });
    candidates_.resize(keep);
  }

  /**
   * Outlines the output neuron with the largest activation.
   *
   * This is the network's answer, and it is worth marking without relying on
   * text, which the renderer cannot draw yet.
   */
  void draw_winner(const std::vector<models::layer> &layers) {
    const auto &output{layers.back().activations};
    const auto count{static_cast<std::size_t>(output.size())};
    if (count == 0) {
      return;
    }

    const auto last{layers.size() - 1};
    const auto radius{std::fmin(spacing(count) * NEURON_FILL, MAX_RADIUS) *
                      WINNER_SCALE};

    renderer_.get().set_color(WINNER_COLOR);
    draw_ring(neuron_position(last, index_of_max(output), count, layers.size()),
              radius);
  }

  /**
   * The NDC position of the center of one neuron.
   *
   * The input layer is laid out as the image it is; every other layer is a
   * vertical column. Connections leave each neuron from exactly this point, so
   * the wiring stays attached to the node it belongs to.
   */
  [[nodiscard]] static Eigen::Vector2f
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  neuron_position(std::size_t column, std::size_t index, std::size_t count,
                  std::size_t columns) {
    if (column == 0) {
      return input_pixel_center(static_cast<std::size_t>(index));
    }

    const auto center{column_center(column, columns)};

    if (count <= 1) {
      return {center, center_y()};
    }

    const auto offset{vertical_span() / static_cast<float>(count - 1)};

    return {center,
            shared::NETWORK_BOTTOM + offset * static_cast<float>(index)};
  }

  /**
   * The NDC x position of the center of a layer's column.
   *
   * The input layer is given a wider slice of the diagram, because it is drawn
   * as a readable image rather than a column of dots.
   */
  [[nodiscard]] static float column_center(std::size_t column,
                                           std::size_t columns) {
    const auto others{columns > 1 ? columns - 1 : 1};
    const auto input_width{diagram_span() * shared::INPUT_COLUMN_SHARE};
    const auto rest_width{diagram_span() - input_width};
    const auto hidden_width{rest_width / static_cast<float>(others)};

    if (column == 0) {
      return shared::NETWORK_LEFT + input_width * HALF;
    }

    return shared::NETWORK_LEFT + input_width +
           hidden_width * (static_cast<float>(column - 1) + HALF);
  }

  /**
   * The NDC center of one input pixel, addressed by its index into the input
   * vector. Row-major from the bottom left, matching how the grid is painted.
   */
  [[nodiscard]] static Eigen::Vector2f input_pixel_center(std::size_t index) {
    const auto side{input_side()};
    const auto across{pixels_across()};
    const auto cell{pixel_offset(index, across)};

    const auto left{shared::NETWORK_LEFT};
    const auto bottom{shared::NETWORK_TOP - side * shared::WINDOW_ASPECT};

    return {left + (cell.x() + HALF) * side / across,
            bottom + (cell.y() + HALF) * side * shared::WINDOW_ASPECT / across};
  }

  /**
   * The bottom left corner of one input pixel.
   */
  [[nodiscard]] static Eigen::Vector2f input_pixel_origin(std::size_t index,
                                                          float cell) {
    const auto across{pixels_across()};
    const auto offset{pixel_offset(index, across)};
    const auto side{input_side()};

    const auto left{shared::NETWORK_LEFT};
    const auto bottom{shared::NETWORK_TOP - side * shared::WINDOW_ASPECT};

    return {left + offset.x() * side / across,
            bottom + offset.y() * side * shared::WINDOW_ASPECT / across};
  }

  /**
   * The row and column of an input pixel within the grid, as floats.
   *
   * Row-major from the bottom left, matching the order the input vector is
   * filled in, so index n is at column n mod 28, row n / 28.
   */
  [[nodiscard]] static Eigen::Vector2f pixel_offset(std::size_t index,
                                                    float across) {
    const auto flat{static_cast<float>(index)};
    return {std::fmod(flat, across), std::floor(flat / across)};
  }

  /** The number of pixels along one side of the input layer. */
  [[nodiscard]] static constexpr float pixels_across() {
    return static_cast<float>(shared::GRID_SIZE);
  }

  /** The width of the whole diagram in NDC. */
  [[nodiscard]] static constexpr float diagram_span() {
    return shared::NETWORK_RIGHT - shared::NETWORK_LEFT;
  }

  /** The height of the whole diagram in NDC. */
  [[nodiscard]] static constexpr float vertical_span() {
    return shared::NETWORK_TOP - shared::NETWORK_BOTTOM;
  }

  /** The vertical midpoint of the diagram in NDC. */
  [[nodiscard]] static constexpr float center_y() {
    return (shared::NETWORK_TOP + shared::NETWORK_BOTTOM) * HALF;
  }

  /**
   * The width of the input image in NDC.
   *
   * The image is drawn in the input layer's slice of the diagram and kept
   * square in pixels, so its height in NDC is its width times the window
   * aspect.
   */
  [[nodiscard]] static float input_side() {
    return diagram_span() * shared::INPUT_COLUMN_SHARE;
  }

  /** The NDC width of one input pixel. */
  [[nodiscard]] static float input_cell() {
    return input_side() / pixels_across();
  }

  /** The vertical spacing available to each neuron in a column. */
  [[nodiscard]] static float spacing(std::size_t count) {
    const auto span{shared::NETWORK_TOP - shared::NETWORK_BOTTOM};
    return count <= 1 ? span : span / static_cast<float>(count - 1);
  }

  /** The largest absolute activation in a layer. */
  [[nodiscard]] static float activation_peak(const models::layer &layer) {
    return layer.activations.cwiseAbs().maxCoeff();
  }

  /** An activation scaled into 0..1 against the peak of its own layer. */
  [[nodiscard]] static float relative_activation(const models::layer &layer,
                                                 std::size_t index,
                                                 float peak) {
    if (peak <= 0.0f) {
      return 0.0f;
    }

    return std::fabs(layer.activations(static_cast<Eigen::Index>(index))) /
           peak;
  }

  /** The index of the largest value in a vector, the first on a tie. */
  [[nodiscard]] static std::size_t index_of_max(const Eigen::VectorXf &v) {
    Eigen::Index best{0};
    for (Eigen::Index i{1}; i < v.size(); i++) {
      if (v(i) > v(best)) {
        best = i;
      }
    }
    return static_cast<std::size_t>(best);
  }

  /**
   * Draws a circle as a fan of triangles, since the renderer has no round
   * primitive.
   */
  void draw_circle(const Eigen::Vector2f &center, float radius) {
    for (int i{0}; i < CIRCLE_SEGMENTS; i++) {
      const auto a{angle(i)};
      const auto b{angle(i + 1)};

      renderer_.get().draw_triangle(center, on_circle(center, radius, a),
                                    on_circle(center, radius, b));
    }
  }

  /**
   * Draws a circle outline, to mark the winning neuron without covering it.
   */
  void draw_ring(const Eigen::Vector2f &center, float radius) {
    for (int i{0}; i < CIRCLE_SEGMENTS; i++) {
      const auto a{angle(i)};
      const auto b{angle(i + 1)};

      renderer_.get().draw_line(on_circle(center, radius, a),
                                on_circle(center, radius, b), RING_THICKNESS);
    }
  }

  [[nodiscard]] static Eigen::Vector2f on_circle(const Eigen::Vector2f &center,
                                                 float radius, float theta) {
    return center +
           Eigen::Vector2f{radius * std::cos(theta), radius * std::sin(theta)};
  }

  [[nodiscard]] static float angle(int segment) {
    return TWO_PI * static_cast<float>(segment) /
           static_cast<float>(CIRCLE_SEGMENTS);
  }

  struct candidate {
    float strength;
    std::size_t source;
  };

  std::reference_wrapper<engine::graphics::renderer> renderer_;

  /** Scratch space reused across neurons, so drawing does not allocate. */
  std::vector<candidate> candidates_{};

  static constexpr std::size_t MIN_LAYERS{2};
  static constexpr std::size_t KEEP_PER_NEURON{16};
  static constexpr float NEURON_FILL{0.4f};
  static constexpr float MAX_RADIUS{0.012f};
  static constexpr float CONNECTION_THICKNESS{0.0016f};
  static constexpr float CONNECTION_GAIN{0.85f};
  static constexpr float WINNER_SCALE{2.4f};
  static constexpr float RING_THICKNESS{0.004f};
  static constexpr float TWO_PI{6.283185307179586f};
  static constexpr float HALF{0.5f};
  static constexpr int CIRCLE_SEGMENTS{8};

  static constexpr Eigen::Vector3f NEURON_COLOR{0.85f, 0.87f, 0.95f};
  static constexpr Eigen::Vector3f POSITIVE_COLOR{0.30f, 0.70f, 1.00f};
  static constexpr Eigen::Vector3f NEGATIVE_COLOR{1.00f, 0.45f, 0.25f};
  static constexpr Eigen::Vector3f WINNER_COLOR{0.25f, 1.00f, 0.55f};
};

} // namespace views
