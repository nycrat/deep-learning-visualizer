#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

namespace dl {

/// Represents matrix data from an IDX file as a one dimensional vector.
///
/// @see https://www.fon.hum.uva.nl/praat/manual/IDX_file_format.html
class idx_matrix {
public:
  /// Constructs an IDX matrix from a file
  explicit idx_matrix(const std::filesystem::path &file_path);

  [[nodiscard]] std::span<const uint8_t> data() const;
  [[nodiscard]] std::size_t rows() const;
  [[nodiscard]] std::size_t cols() const;

private:
  std::size_t rows_{};
  std::size_t cols_{};
  std::vector<uint8_t> data_{};
};

} // namespace dl
