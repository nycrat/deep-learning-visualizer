#pragma once

/** @file */

#include "engine/graphics/renderer.h"
#include "models/layer.h"
#include "views/network_view.h"

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
  /**
   * Constructs a new network view.
   *
   * @param renderer The renderer to submit shapes to.
   */
  explicit network_view_gl(engine::graphics::renderer &renderer);

  void draw_network(const std::vector<models::layer> &layers) override;

private:
  /**
   * Draws the input layer as a grid of lit pixels.
   *
   * @param layer The input layer to draw.
   */
  void draw_input_layer(const models::layer &layer);

  /**
   * Draws one layer as a vertical column of circles, one per neuron.
   *
   * Brightness follows the activation, normalized against the largest
   * activation in the same layer, so a column stays legible however large its
   * values happen to grow.
   *
   * @param layer The layer to draw.
   * @param column Which column of the diagram the layer occupies.
   * @param columns How many columns the diagram has in total.
   */
  void draw_neurons(const models::layer &layer, std::size_t column,
                    std::size_t columns);

  /**
   * Draws the strongest connections between two adjacent layers.
   *
   * For each neuron in the destination layer the connections are ranked by
   * |weight| times the activation of the neuron feeding them, and only the top
   * few are drawn. Multiplying by the source activation is what makes the
   * picture track the input: a large weight attached to a dark pixel
   * contributes nothing, and is correctly drawn faintly or not at all.
   *
   * @param source The layer the connections come from.
   * @param dest The layer the connections go to.
   * @param column Which column of the diagram the destination occupies.
   * @param columns How many columns the diagram has in total.
   */
  void draw_connections(const models::layer &source, const models::layer &dest,
                        std::size_t column, std::size_t columns);

  /**
   * Ranks the connections into one destination neuron, strongest first, and
   * leaves the top KEEP_PER_NEURON of them in candidates_.
   *
   * Reuses the scratch vector so that drawing does not allocate per neuron.
   *
   * @param source The layer the connections come from.
   * @param dest The layer the connections go to.
   * @param destination The index of the neuron being fed.
   */
  void rank_incoming(const models::layer &source, const models::layer &dest,
                     std::size_t destination);

  /**
   * Outlines the output neuron with the largest activation.
   *
   * This is the network's answer, and it is worth marking without relying on
   * text, which the renderer cannot draw yet.
   *
   * @param layers Every layer of the network, in order.
   */
  void draw_winner(const std::vector<models::layer> &layers);

  /**
   * The NDC position of the center of one neuron.
   *
   * The input layer is laid out as the image it is; every other layer is a
   * vertical column. Connections leave each neuron from exactly this point, so
   * the wiring stays attached to the node it belongs to.
   *
   * @param column Which column of the diagram the neuron is in.
   * @param index Which neuron within the layer.
   * @param count How many neurons the layer has.
   * @param columns How many columns the diagram has in total.
   *
   * @return The position in NDC.
   */
  [[nodiscard]] static Eigen::Vector2f neuron_position(std::size_t column,
                                                       std::size_t index,
                                                       std::size_t count,
                                                       std::size_t columns);

  /**
   * The NDC x position of the center of a layer's column.
   *
   * The input layer is given a wider slice of the diagram, because it is drawn
   * as a readable image rather than a column of dots.
   *
   * @param column Which column of the diagram to place.
   * @param columns How many columns the diagram has in total.
   *
   * @return The x position in NDC.
   */
  [[nodiscard]] static float column_center(std::size_t column,
                                           std::size_t columns);

  /**
   * The NDC center of one input pixel, addressed by its index into the input
   * vector. Row-major from the bottom left, matching how the grid is painted.
   *
   * @param index The index of the pixel in the input vector.
   *
   * @return The position in NDC.
   */
  [[nodiscard]] static Eigen::Vector2f input_pixel_center(std::size_t index);

  /**
   * The bottom left corner of one input pixel.
   *
   * @param index The index of the pixel in the input vector.
   * @param cell The NDC width of one input pixel.
   *
   * @return The position in NDC.
   */
  [[nodiscard]] static Eigen::Vector2f input_pixel_origin(std::size_t index,
                                                          float cell);

  /**
   * The row and column of an input pixel within the grid, as floats.
   *
   * Row-major from the bottom left, matching the order the input vector is
   * filled in, so index n is at column n mod 28, row n / 28.
   *
   * @param index The index of the pixel in the input vector.
   * @param across How many pixels lie along one side of the grid.
   *
   * @return The column and row of the pixel.
   */
  [[nodiscard]] static Eigen::Vector2f pixel_offset(std::size_t index,
                                                    float across);

  /** The number of pixels along one side of the input layer. */
  [[nodiscard]] static float pixels_across();

  /** The width of the whole diagram in NDC. */
  [[nodiscard]] static float diagram_span();

  /** The height of the whole diagram in NDC. */
  [[nodiscard]] static float vertical_span();

  /** The vertical midpoint of the diagram in NDC. */
  [[nodiscard]] static float center_y();

  /**
   * The width of the input image in NDC.
   *
   * The image is drawn in the input layer's slice of the diagram and kept
   * square in pixels, so its height in NDC is its width times the window
   * aspect.
   *
   * @return The width in NDC.
   */
  [[nodiscard]] static float input_side();

  /** The NDC width of one input pixel. */
  [[nodiscard]] static float input_cell();

  /**
   * The vertical spacing available to each neuron in a column.
   *
   * @param count How many neurons the column holds.
   *
   * @return The spacing in NDC.
   */
  [[nodiscard]] static float spacing(std::size_t count);

  /**
   * The largest absolute activation in a layer.
   *
   * @param layer The layer to measure.
   *
   * @return The peak activation.
   */
  [[nodiscard]] static float activation_peak(const models::layer &layer);

  /**
   * An activation scaled into 0..1 against the peak of its own layer.
   *
   * @param layer The layer the neuron belongs to.
   * @param index Which neuron within the layer.
   * @param peak The peak activation of the layer.
   *
   * @return The activation, normalized.
   */
  [[nodiscard]] static float relative_activation(const models::layer &layer,
                                                 std::size_t index, float peak);

  /**
   * The index of the largest value in a vector, the first on a tie.
   *
   * @param v The vector to search.
   *
   * @return The index of the largest value.
   */
  [[nodiscard]] static std::size_t index_of_max(const Eigen::VectorXf &v);

  /**
   * Draws a circle as a fan of triangles, since the renderer has no round
   * primitive.
   *
   * @param center The center of the circle in NDC.
   * @param radius The radius in NDC.
   */
  void draw_circle(const Eigen::Vector2f &center, float radius);

  /**
   * Draws a circle outline, to mark the winning neuron without covering it.
   *
   * @param center The center of the ring in NDC.
   * @param radius The radius in NDC.
   */
  void draw_ring(const Eigen::Vector2f &center, float radius);

  /**
   * A point on the circumference of a circle.
   *
   * @param center The center of the circle in NDC.
   * @param radius The radius in NDC.
   * @param theta The angle around the circle in radians.
   *
   * @return The position in NDC.
   */
  [[nodiscard]] static Eigen::Vector2f on_circle(const Eigen::Vector2f &center,
                                                 float radius, float theta);

  /**
   * The angle of one segment of a circle approximation.
   *
   * @param segment Which segment around the circle.
   *
   * @return The angle in radians.
   */
  [[nodiscard]] static float angle(int segment);

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
