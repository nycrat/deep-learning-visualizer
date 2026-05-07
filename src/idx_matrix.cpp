#include "idx_matrix.h"

#include <array>
#include <format>
#include <fstream>
#include <functional>
#include <numeric>
#include <stdexcept>

namespace avah {

idx_matrix::idx_matrix(const std::filesystem::path &file_path) {
  std::ifstream file{file_path, std::ios::binary};

  if (!file) {
    throw std::runtime_error(
        std::format("Failed to open {}", file_path.string()));
  }

  // Format is 0x00 0x00 (data type) (number of dimensions)
  std::array<std::byte, 4> magic{};
  file.read(reinterpret_cast<char *>(magic.data()), magic.size());

  // Currently only using unsigned bytes for this project
  [[maybe_unused]] auto data_type = static_cast<std::size_t>(magic.at(2));
  auto dimensions = static_cast<std::size_t>(magic.at(3));

  std::vector<std::size_t> dimension_sizes(dimensions, 0);

  for (std::size_t &size : dimension_sizes) {
    std::array<std::byte, 4> dimension_size_buffer{};
    file.read(reinterpret_cast<char *>(dimension_size_buffer.data()),
              dimension_size_buffer.size());

    for (std::size_t j{0}; j < 4; j++) {
      size *= 256;
      size += static_cast<std::size_t>(dimension_size_buffer.at(j));
    }
  }

  // Special case for 1 dimension, read as row vector
  if (dimensions == 1) {
    rows_ = 1;
    cols_ = dimension_sizes[0];
  } else {
    rows_ = dimension_sizes[0];
    cols_ = std::accumulate(dimension_sizes.begin() + 1, dimension_sizes.end(),
                            std::size_t{1}, std::multiplies<>());
  }

  data_ = std::vector<uint8_t>(rows_ * cols_);
  file.read(reinterpret_cast<char *>(this->data_.data()), rows_ * cols_);
}

std::span<const uint8_t> idx_matrix::data() const {
  return data_;
}

std::size_t idx_matrix::rows() const {
  return rows_;
}

std::size_t idx_matrix::cols() const {
  return cols_;
}

} // namespace avah
