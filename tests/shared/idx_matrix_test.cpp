#include "catch_amalgamated.h"
#include "shared/idx_matrix.h"
#include "test_helpers.h"

#include <cstdint>
#include <filesystem>
#include <ranges>
#include <stdexcept>
#include <vector>

namespace {

using test_helpers::idx_header;
using test_helpers::temp_file;

std::vector<std::uint8_t>
make_idx_file(const std::vector<std::int64_t> &dimensions,
              const std::vector<std::uint8_t> &data) {
  auto bytes{idx_header(0x08, dimensions)};
  bytes.insert(bytes.end(), data.begin(), data.end());
  return bytes;
}

} // namespace

TEST_CASE("Test idx_matrix constructor reads a two dimensional file") {
  const temp_file file{"idx_matrix_2d"};

  const std::vector<std::uint8_t> data{1, 2, 3, 4, 5, 6};
  file.write_bytes(make_idx_file({2, 3}, data));

  const shared::idx_matrix matrix{file.path()};

  CHECK(matrix.rows() == 2);
  CHECK(matrix.cols() == 3);
  CHECK(matrix.data() == data);
}

TEST_CASE("Test idx_matrix constructor flattens the trailing dimensions") {
  const temp_file file{"idx_matrix_3d"};

  const std::vector<std::uint8_t> data(2 * 28 * 28, 7);
  file.write_bytes(make_idx_file({2, 28, 28}, data));

  const shared::idx_matrix matrix{file.path()};

  CHECK(matrix.rows() == 2);
  CHECK(matrix.cols() == 28 * 28);
  CHECK(matrix.data() == data);
}

TEST_CASE("Test idx_matrix constructor reads a vector file") {
  const temp_file file{"idx_matrix_1d"};

  const std::vector<std::uint8_t> data{5, 1, 9, 4, 1, 9, 4, 1};
  file.write_bytes(make_idx_file({8}, data));

  const shared::idx_matrix matrix{file.path()};

  CHECK(matrix.rows() == 8);
  CHECK(matrix.cols() == 1);
  CHECK(matrix.data() == data);
}

TEST_CASE("Test idx_matrix constructor reads the full byte range") {
  const temp_file file{"idx_matrix_bytes"};

  std::vector<std::uint8_t> data(256);
  for (std::size_t i{0}; i < data.size(); i++) {
    data.at(i) = static_cast<std::uint8_t>(i);
  }
  file.write_bytes(make_idx_file({256}, data));

  const shared::idx_matrix matrix{file.path()};

  CHECK(matrix.rows() == 256);
  CHECK(matrix.cols() == 1);
  CHECK(matrix.data() == data);
}

TEST_CASE("Test idx_matrix constructor reads big endian dimensions") {
  const temp_file file{"idx_matrix_dimensions"};

  const std::vector<std::int64_t> dimensions{60000, 1};
  const std::vector<std::uint8_t> data(60000, 3);
  file.write_bytes(make_idx_file(dimensions, data));

  const shared::idx_matrix matrix{file.path()};

  CHECK(matrix.rows() == 60000);
  CHECK(matrix.cols() == 1);
  CHECK(matrix.data() == data);
}

TEST_CASE("Test idx_matrix constructor throws when the file is missing") {
  CHECK_THROWS_AS(shared::idx_matrix{std::filesystem::path{"no_such_file.idx"}},
                  std::runtime_error);
}

TEST_CASE("Test idx_matrix constructor reads the mnist data set") {
  const std::filesystem::path labels_path{"data/mnist/test-labels.idx"};
  const std::filesystem::path images_path{"data/mnist/test-images.idx"};

  if (!std::filesystem::exists(labels_path) ||
      !std::filesystem::exists(images_path)) {
    SKIP("the mnist data set is not available");
  }

  const shared::idx_matrix labels{labels_path};
  CHECK(labels.rows() == 10000);
  CHECK(labels.cols() == 1);
  CHECK(labels.data().size() == 10000);
  CHECK(labels.data().at(0) < 10);

  const shared::idx_matrix images{images_path};
  CHECK(images.rows() == 10000);
  CHECK(images.cols() == 28 * 28);
  CHECK(images.data().size() == 10000 * 28 * 28);
  CHECK(*std::ranges::max_element(images.data()) > 0);
}
