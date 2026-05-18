#include "idx_matrix.h"

#include <format>
#include <fstream>
#include <functional>
#include <ios>
#include <numeric>
#include <ranges>
#include <stdexcept>

namespace {

template <typename T>
std::vector<T> read(std::ifstream &stream, std::uint64_t size) {
  std::vector<char> buf(size);
  stream.read(buf.data(), static_cast<std::streamsize>(size));

  return buf | std::views::transform([](char c) {
           return static_cast<T>(static_cast<unsigned char>(c));
         }) |
         std::ranges::to<std::vector<T>>();
}

} // namespace

namespace shared {

idx_matrix::idx_matrix(const std::filesystem::path &file_path) {
  std::ifstream file{file_path, std::ios::binary};

  if (!file) {
    throw std::runtime_error(
        std::format("Failed to open {}", file_path.string()));
  }

  // Format is 0x00 0x00 (data type) (number of dimensions)
  auto magic = read<std::int64_t>(file, 4);

  // Currently only using unsigned bytes for this project
  [[maybe_unused]] auto data_type = magic.at(2);
  auto num_dimensions = magic.at(3);

  std::vector<std::int64_t> dimension_sizes(num_dimensions);

  for (auto &size : dimension_sizes) {
    auto dimension_size_buffer = read<std::int64_t>(file, 4);

    for (int j{0}; j < 4; j++) {
      size *= 256;
      size += dimension_size_buffer.at(j);
    }
  }

  rows_ = dimension_sizes.at(0);
  cols_ = std::accumulate(dimension_sizes.begin() + 1, dimension_sizes.end(),
                          std::int64_t{1}, std::multiplies<>());
  data_ = read<std::uint8_t>(file, rows_ * cols_);
}

const std::vector<uint8_t> &idx_matrix::data() const {
  return data_;
}

std::int64_t idx_matrix::rows() const {
  return rows_;
}

std::int64_t idx_matrix::cols() const {
  return cols_;
}

} // namespace shared
