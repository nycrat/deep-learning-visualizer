#pragma once

/**
 * @file
 * @brief Utilities shared by the unit tests.
 */

#include <catch_amalgamated.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace test_helpers {

/**
 * A scratch file in the system temporary directory which is removed once the
 * test finishes, whether it passed or not.
 */
class temp_file {
public:
  /**
   * @param name A suffix which keeps the files of separate tests apart.
   */
  explicit temp_file(std::string_view name)
      : path_{std::filesystem::temp_directory_path() /
              (std::string{"deep_learning_visualizer_"} + std::string{name} +
               ".tmp")} {
    std::filesystem::remove(path_);
  }

  ~temp_file() {
    std::filesystem::remove(path_);
  }

  temp_file(const temp_file &) = delete;
  temp_file &operator=(const temp_file &) = delete;
  temp_file(temp_file &&) = delete;
  temp_file &operator=(temp_file &&) = delete;

  /**
   * Replaces the contents of the file with the given text.
   */
  void write_text(std::string_view text) const {
    std::ofstream file{path_};
    file << text;
  }

  /**
   * Replaces the contents of the file with the given raw bytes.
   */
  void write_bytes(const std::vector<std::uint8_t> &bytes) const {
    std::ofstream file{path_, std::ios::binary};
    file.write(reinterpret_cast<const char *>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
  }

  /**
   * @return The path of the scratch file.
   */
  [[nodiscard]] const std::filesystem::path &path() const {
    return path_;
  }

private:
  std::filesystem::path path_;
};

/**
 * Builds the header of an IDX file, which is a null magic number, a one byte
 * data type, a one byte dimension count, and then the big endian size of each
 * dimension.
 *
 * @param data_type The type of the stored data, such as 0x08 for a byte.
 * @param dimensions The size of each dimension of the data.
 *
 * @return The bytes which precede the data in an IDX file.
 */
inline std::vector<std::uint8_t>
idx_header(std::uint8_t data_type,
           const std::vector<std::int64_t> &dimensions) {
  std::vector<std::uint8_t> header{
      0x00, 0x00, data_type, static_cast<std::uint8_t>(dimensions.size())};

  for (auto dimension : dimensions) {
    for (int shift{24}; shift >= 0; shift -= 8) {
      header.push_back(static_cast<std::uint8_t>(
          (static_cast<std::uint64_t>(dimension) >> shift) & 0xFFU));
    }
  }

  return header;
}

/**
 * Matches a float against a target within an absolute tolerance.
 *
 * @param target The expected value.
 * @param epsilon The largest accepted absolute difference.
 *
 * @return A matcher for use with CHECK_THAT and REQUIRE_THAT.
 */
inline Catch::Matchers::WithinAbsMatcher within_abs(float target,
                                                    double epsilon = 1e-5) {
  return Catch::Matchers::WithinAbs(static_cast<double>(target), epsilon);
}

} // namespace test_helpers
