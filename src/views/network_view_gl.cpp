#include "views/network_view_gl.h"

#include "shared/layout.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

namespace views {

network_view_gl::network_view_gl(engine::graphics::renderer &renderer)
    : renderer_(renderer) {
}

void network_view_gl::draw_network(const std::vector<models::layer> &layers) {
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

void network_view_gl::draw_input_layer(const models::layer &layer) {
  const auto cell{input_cell()};

  for (Eigen::Index i{0}; i < layer.activations.size(); i++) {
    const auto value{layer.activations(i)};
    if (value <= 0.0f) {
      continue;
    }

    const auto anchor{input_pixel_origin(static_cast<std::size_t>(i), cell)};

    renderer_.get().set_color(
        Eigen::Vector4f{1.0f, 1.0f, 1.0f, std::fmin(value, 1.0f)});
    renderer_.get().draw_quad(
        anchor, anchor + Eigen::Vector2f{cell, 0.0f},
        anchor + Eigen::Vector2f{cell, cell * shared::WINDOW_ASPECT},
        anchor + Eigen::Vector2f{0.0f, cell * shared::WINDOW_ASPECT});
  }
}

void network_view_gl::draw_neurons(const models::layer &layer,
                                   std::size_t column, std::size_t columns) {
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

void network_view_gl::draw_connections(const models::layer &source,
                                       const models::layer &dest,
                                       std::size_t column,
                                       std::size_t columns) {
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
          neuron_position(column - 1, candidate.source, source_n, columns), to,
          CONNECTION_THICKNESS);
    }
  }
}

void network_view_gl::rank_incoming(const models::layer &source,
                                    const models::layer &dest,
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

void network_view_gl::draw_winner(const std::vector<models::layer> &layers) {
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

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
Eigen::Vector2f network_view_gl::neuron_position(std::size_t column,
                                                 std::size_t index,
                                                 std::size_t count,
                                                 std::size_t columns) {
  if (column == 0) {
    return input_pixel_center(index);
  }

  const auto center{column_center(column, columns)};

  if (count <= 1) {
    return {center, center_y()};
  }

  const auto offset{vertical_span() / static_cast<float>(count - 1)};

  return {center, shared::NETWORK_BOTTOM + offset * static_cast<float>(index)};
}

float network_view_gl::column_center(std::size_t column, std::size_t columns) {
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

Eigen::Vector2f network_view_gl::input_pixel_center(std::size_t index) {
  const auto side{input_side()};
  const auto across{pixels_across()};
  const auto cell{pixel_offset(index, across)};

  const auto left{shared::NETWORK_LEFT};
  const auto bottom{shared::NETWORK_TOP - side * shared::WINDOW_ASPECT};

  return {left + (cell.x() + HALF) * side / across,
          bottom + (cell.y() + HALF) * side * shared::WINDOW_ASPECT / across};
}

Eigen::Vector2f network_view_gl::input_pixel_origin(std::size_t index,
                                                    float cell) {
  const auto across{pixels_across()};
  const auto offset{pixel_offset(index, across)};
  const auto side{input_side()};

  const auto left{shared::NETWORK_LEFT};
  const auto bottom{shared::NETWORK_TOP - side * shared::WINDOW_ASPECT};

  return {left + offset.x() * side / across,
          bottom + offset.y() * side * shared::WINDOW_ASPECT / across};
}

Eigen::Vector2f network_view_gl::pixel_offset(std::size_t index, float across) {
  const auto flat{static_cast<float>(index)};
  return {std::fmod(flat, across), std::floor(flat / across)};
}

float network_view_gl::pixels_across() {
  return static_cast<float>(shared::GRID_SIZE);
}

float network_view_gl::diagram_span() {
  return shared::NETWORK_RIGHT - shared::NETWORK_LEFT;
}

float network_view_gl::vertical_span() {
  return shared::NETWORK_TOP - shared::NETWORK_BOTTOM;
}

float network_view_gl::center_y() {
  return (shared::NETWORK_TOP + shared::NETWORK_BOTTOM) * HALF;
}

float network_view_gl::input_side() {
  return diagram_span() * shared::INPUT_COLUMN_SHARE;
}

float network_view_gl::input_cell() {
  return input_side() / pixels_across();
}

float network_view_gl::spacing(std::size_t count) {
  const auto span{shared::NETWORK_TOP - shared::NETWORK_BOTTOM};
  return count <= 1 ? span : span / static_cast<float>(count - 1);
}

float network_view_gl::activation_peak(const models::layer &layer) {
  return layer.activations.cwiseAbs().maxCoeff();
}

float network_view_gl::relative_activation(const models::layer &layer,
                                           std::size_t index, float peak) {
  if (peak <= 0.0f) {
    return 0.0f;
  }

  return std::fabs(layer.activations(static_cast<Eigen::Index>(index))) / peak;
}

std::size_t network_view_gl::index_of_max(const Eigen::VectorXf &v) {
  Eigen::Index best{0};
  for (Eigen::Index i{1}; i < v.size(); i++) {
    if (v(i) > v(best)) {
      best = i;
    }
  }
  return static_cast<std::size_t>(best);
}

void network_view_gl::draw_circle(const Eigen::Vector2f &center, float radius) {
  for (int i{0}; i < CIRCLE_SEGMENTS; i++) {
    const auto a{angle(i)};
    const auto b{angle(i + 1)};

    renderer_.get().draw_triangle(center, on_circle(center, radius, a),
                                  on_circle(center, radius, b));
  }
}

void network_view_gl::draw_ring(const Eigen::Vector2f &center, float radius) {
  for (int i{0}; i < CIRCLE_SEGMENTS; i++) {
    const auto a{angle(i)};
    const auto b{angle(i + 1)};

    renderer_.get().draw_line(on_circle(center, radius, a),
                              on_circle(center, radius, b), RING_THICKNESS);
  }
}

Eigen::Vector2f network_view_gl::on_circle(const Eigen::Vector2f &center,
                                           float radius, float theta) {
  return center +
         Eigen::Vector2f{radius * std::cos(theta), radius * std::sin(theta)};
}

float network_view_gl::angle(int segment) {
  return TWO_PI * static_cast<float>(segment) /
         static_cast<float>(CIRCLE_SEGMENTS);
}

} // namespace views
