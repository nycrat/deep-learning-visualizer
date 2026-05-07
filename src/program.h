#pragma once

#include <cstdint>
#include <filesystem>

namespace dl_visualizer {

/// Represents an OpenGL shader program.
class program {
public:
  /// Constructs a program that compiles and links a vertex and fragment shader.
  program(const std::filesystem::path &vertex_shader_path,
          const std::filesystem::path &fragment_shader_path);
  ~program();
  program(const program &) = delete;
  program &operator=(const program &) = delete;
  program(program &&) = delete;
  program &operator=(program &&) = delete;

  /// Installs this program in the current OpenGL context.
  void use();

  [[nodiscard]] std::uint32_t id() const;

private:
  std::uint32_t id_;
};

} // namespace dl_visualizer
